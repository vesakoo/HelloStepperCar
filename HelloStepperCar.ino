#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "SSID.h"
#include <ArduinoHttpClient.h>
#include "ArduinoLowPower.h"

#include <AccelStepper.h>
#include <NewPing.h>

// sonar pins
#define TRIGGER_PIN_FRONT  11  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN_FRONT     12  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
#define TRIGGER_PIN_REAR  18 
#define ECHO_PIN_REAR 19
//sonar led pins
#define LED_RED 7
#define LED_YELLOW 6
#define LED_GREEN 5
//power
#define SLEEP_10S_AFTER 180000ul
#define SLEEP_15S_AFTER 300000ul
#define SLEEP_30S_AFTER 600000ul
#define SLEEP_60S_AFTER 1800000ul
#define SLEEP_120S_AFTER 3600000ul

/// @Power save //////////
unsigned long lastRunTime =0l;
unsigned long lastWakeUpTime =0l;

/// wifi params

AccelStepper leftStepper(AccelStepper::FULL4WIRE, 4,6, 5, 7);
AccelStepper rightStepper(AccelStepper::FULL4WIRE, 8, 10, 9, 11);
NewPing sonarFront(TRIGGER_PIN_FRONT, ECHO_PIN_FRONT, MAX_DISTANCE);
NewPing sonarRear(TRIGGER_PIN_REAR, ECHO_PIN_REAR, MAX_DISTANCE);


/// @http parts ////
//char server[] = "robo.sukelluspaikka.fi";
IPAddress server(192,168,32,87); 
WiFiClient cli;
HttpClient client = HttpClient(cli,server,3002);
int actionNum=0;
bool seqEndReported = false;
char deviceId [] = "xxx-yyy-zzz";  //unique id for this robot

void postSeqEnd(){
  String contentType = "application/json";
  String postData = "{\"seq\":\"end\"}";
  char str[80];
  sprintf(str, "/robot/%s/seq/1/end", deviceId);
  puts(str);
  client.post(str,contentType, postData);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void setupWifi(){
  int status;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(SECRET_SSID, SECRET_PASS);
    delay(10000);
  }
  client.setTimeout(3000);
  printWiFiStatus();
}

////////////////

////////////////
bool trafficLight(bool isFront =false, bool accurate =false){
  int sonar_cm = 0;
  if(isFront){
    sonar_cm = (int) sonarFront.ping_cm();
    //log("Sonar front:" + (String) sonar_cm + " cm");
  }else{
    sonar_cm = (int) sonarRear.ping_cm();
    //log("Sonar back:" + (String) sonar_cm + " cm");
  }
  digitalWrite(LED_GREEN,LOW);
  digitalWrite(LED_YELLOW,LOW);
  digitalWrite(LED_RED,LOW);
  if(sonar_cm >0 && sonar_cm < 20){
    digitalWrite(LED_RED,HIGH);
    return true;
  }else if (sonar_cm >0 && sonar_cm < 100){
    digitalWrite(LED_YELLOW,HIGH);
  }else{
    digitalWrite(LED_GREEN,HIGH);
  }
  return false;
}
///////////////

/// @Robot actions ////
/*void curve(int radius,int speed, int angle){
  int carWidth = 10;
  float innerDist = ((float)radius - (float)carWidth/2.0)*TWO_PI*(float)angle/360.0;
  float outerDist = -1.0*((float)radius - (float)carWidth/2.0)*TWO_PI*(float)angle/360.0;
  float timeTaken = innerDist*(float)speed;
  innerDist = _distanceToSteps(innerDist);
  outerDist = _distanceToSteps(outerDist);
  float innerSpeed = innerDist/timeTaken;
  float outerSpeed = outerDist/timeTaken;
  // float deltaAccel = innerDist/outerDist;
  leftStepper.setSpeed(innerSpeed);
  rightStepper.setSpeed(outerSpeed);
  while(leftStepper.distanceToGo()){
    leftStepper.runSpeed();
    rightStepper.runSpeed();
  }
}*/

void _runLRA(
  float speedLeft,
  float speedRight,
  float distL, 
  float distR,
  float accelL,
  float accelR
){
  /*Serial.print("runLRA(");
  Serial.print(speedLeft);
    Serial.print(",");
  Serial.print(speedRight);
    Serial.print(",");
  Serial.print(distL);
    Serial.print(",");
  Serial.print(distR);
    Serial.print(",");
  Serial.print(accelL);
    Serial.print(",");
  Serial.print(accelR);
  Serial.println("");*/

  //leftStepper.setAcceleration(accelL);
  //rightStepper.setAcceleration(accelR);
  leftStepper.setAcceleration(speedLeft);
  rightStepper.setAcceleration(speedRight);
  _runLR(speedLeft,speedRight,distL,distR);
}

void _runLR(
  int speedLeft,
  int speedRight,
  float stepsL, 
  float stepsR
){
  leftStepper.setMaxSpeed(speedLeft);
  rightStepper.setMaxSpeed(speedRight);
  //leftStepper.setSpeed(speedLeft);
  //rightStepper.setSpeed(speedRight);
  //float slower = speedLeft;
  //float faster = speedRight;
  if(speedLeft > speedRight){
    stepsR = speedFixedStepsForSlower(speedRight,speedLeft,-1.0*stepsL); 
  }else if (speedRight < speedLeft){
    stepsL = speedFixedStepsForSlower(speedLeft,speedRight,-1.0*stepsR);

  }
  leftStepper.moveTo(stepsL);
  rightStepper.moveTo(stepsR);
  
  long ltdg= leftStepper.distanceToGo();
  long rtdg = rightStepper.distanceToGo();
  while(ltdg !=0 && rtdg != 0){
    ltdg= leftStepper.distanceToGo(); 
    rtdg = rightStepper.distanceToGo();
    //rightStepper.setSpeed(speedRight);
    //leftStepper.setSpeed(speedLeft);
    if(_stepWithControls(ltdg !=0 , rtdg !=0)){
      leftStepper.stop();
      rightStepper.stop();
      break;
    }
  }
  leftStepper.setCurrentPosition(0);
  rightStepper.setCurrentPosition(0);
  lastRunTime = lastWakeUpTime+ millis();
}

bool _stepWithControls(bool runL,bool runR){
  if(runL){
    leftStepper.run();
  }
  if(runR){
    rightStepper.run();
  }
  //return trafficLight(true,true);
  return false;
}

////////////////
//////@utils/////////
float _distanceToSteps(float cm){
  float circumference = 17.28; 
  long stepsPerRew = 2048;
  return stepsPerRew * cm/circumference;
}

/// @brief stepperAccel takes n steps per second
// steps/time =speed => steps = speed*time
// time = steps/speed
/// @param slowerSpd 
/// @param fasterSpd 
/// @param steps 
/// @return 
float speedFixedStepsForSlower(float slowerSpd,float fasterSpd,float steps){
  float runTime = steps/fasterSpd;
  return (slowerSpd * runTime);

}

void log(char msg[]){
  Serial.println(msg);
}
/////////

void test(){
  //curve(100,550,90);
  long stepsL = _distanceToSteps(150);
  long stepsR = -1l* _distanceToSteps(150);
  Serial.print(stepsL);
  Serial.print("\t");
  Serial.print(stepsR);
  _runLRA(600,600,stepsL,stepsR,100,100);
  
}

void setup() {

    /*leftStepper.setMaxSpeed(600.0);
    leftStepper.setAcceleration(100.0);
    leftStepper.moveTo(24000);
    
    rightStepper.setMaxSpeed(600.0);
    rightStepper.setAcceleration(100.0);
    rightStepper.moveTo(-24000);*/
    Serial.begin(115200);
    //test();
 
  setupWifi();
}

void loop() {
  WiFi.noLowPowerMode();
  String s = "/robot/" + (String)deviceId +"/action/"+(String)actionNum;
  client.get(s);
  int statusCode = client.responseStatusCode();
  String action = client.responseBody();
  WiFi.lowPowerMode();
  /*Serial.println("<Loop Action is:" +action +">");*/

  int str_len = action.length() + 1; 
  char char_array[str_len]; 
  action.toCharArray(char_array, str_len);
    


  char* ptr = strtok(char_array, "/");
  int radius = 0,
      angle = 0,
      speedAvg = 0,
      speedL = 0,
      speedR = 0,
      distance = 0;
  while (ptr) {
    /*if(strcmp(ptr,"radius")==0){
      ptr = strtok(NULL, "/");
      radius=atoi(ptr);
      ptr = strtok(NULL, "/");
      
    } else if(strcmp(ptr,"angle")==0){
      ptr = strtok(NULL, "/");
      angle = atoi(ptr); //read int

    } else */
    if(strcmp(ptr,"speed")==0){
      ptr = strtok(NULL, "/");
      speedAvg = atoi(ptr); //read int

    } else if(strcmp(ptr,"speedR")==0){
      ptr = strtok(NULL, "/");
      speedR= atoi(ptr); //read int

    } else if(strcmp(ptr,"speedL")==0){
      ptr = strtok(NULL, "/");
      speedL= atoi(ptr); //read int
    }else if(strcmp(ptr,"dist")==0){
      ptr = strtok(NULL, "/");
      distance= atoi(ptr); //read int
    }
     ptr = strtok(NULL, "/"); 
  }
  
  
  if(speedL !=0 && speedR !=0){ //different speed same dist
    distance=_distanceToSteps(distance);
    _runLRA(speedL,speedR,distance,-1.0*distance,speedL/5,speedR/5);
  }else if(distance !=0 && speedAvg !=0){//same speed dist
     distance=_distanceToSteps(distance);
    _runLRA(speedAvg,speedAvg,distance,-1.0 * distance,speedAvg/5,speedAvg/5);
  }

   // put your main code here, to run repeatedly:


  if(action.endsWith("/seq/end")){
    if(!seqEndReported){
        postSeqEnd();
        seqEndReported = true;
    }
    actionNum =0;
  }else{
    seqEndReported =false;
    actionNum++; 
  }
  action ="";
  //leftStepper.run();
  //rightStepper.run(); 
  //if seq/end many times, goto sleep for a while
    unsigned long now = millis();
    /*if(now<lastWakeUpTime){
      now = lastWakeUpTime+now; //now will go out of bounds in 46 days...
    }
    if (now > lastRunTime + SLEEP_120S_AFTER ){
     log("LOW POWER 120S");
     lastWakeUpTime =now +120000;
     LowPower.sleep(120000);
     lastRunTime = now - SLEEP_120S_AFTER; //so that lastRuntime dont go out of bounds
    }
    else if(now > lastRunTime + SLEEP_60S_AFTER  ){
      log("LOW POWER 60S");
      lastWakeUpTime =now +60000;
      LowPower.sleep(60000);
    }
    else if(now > lastRunTime + SLEEP_30S_AFTER  ){
      log("LOW POWER 30S");
      lastWakeUpTime =now + 30000;
      LowPower.sleep(30000);
    }
    else if(now > lastRunTime + SLEEP_15S_AFTER  ){
      log("LOW POWER 15S");
      lastWakeUpTime =now +15000;
      LowPower.sleep(15000);
    }
    else if(now > lastRunTime + SLEEP_10S_AFTER  ){ //after 3 mins of idling sleep a second
      log("LOW POWER 10S");
      lastWakeUpTime =now + 10000;
      LowPower.sleep(10000);

    }*/
}
