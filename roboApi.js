const api_stepper =[
  {cmd: '/speed/500/dist/100',label: 'fwd',description: 'fwd 1m'},
  {cmd: '/speed/500/dist/-100',label: 'bwd',description: 'bwd 1m'},
  {cmd: '/speedL/500/speedR/600/dist/150',label: 'curve L',description: 'curve l'},
  {cmd: '/speedL/600/speedR/500/dist/150',label: 'curve R',description: 'curve R'},
  {cmd: '/speedL/500/speedR/600/dist/-150',label: 'bwd curve L',description: 'bwd curve l'},
  {cmd: '/speedL/600/speedR/500/dist/-150',label: 'bwd curve R',description: 'bwd curve R'},
]

const testdataStepper ={
  deviceName: "4tronix Initio stepper",
  currentSeqvenceId: 1,
  deviceId: 'xxx-yyy-zzz',
  secretApiKey: 'xxx-yyy-zzz', 
  inManualMode: false,
  version: '1.0',
  //lastExecutedRow: 0,
  currentSequenceName: "None",
  currentSequenceId: "None",
  currentSequence: [],
  projectQue: []

}

const deviceStack ={
  deviceId: 'xxx-yyy-zzz',
  secretApiKey: 'secretApiKey', 
  stack: testdataStepper
}

const deviceState = 
  {
    deviceId: 'xxx-yyy-zzz',
    secretApiKey: 'secretApiKey', 
    newProjectShifted: true
  }


const deviceApi = 
  {
    deviceId: 'xxx-yyy-zzz',
    secretApiKey: 'xxx-yyy-zzz',
    api: api_stepper
  }

