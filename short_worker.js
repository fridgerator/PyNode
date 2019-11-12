const { parentPort } = require('worker_threads')
const pynode = require('./build/Release/PyNode')

const shortRunningFunction = () => {
  return new Promise((resolve, reject) => {
    pynode.startInterpreter()
    pynode.appendSysPath('./test_files')
    pynode.openFile('tools')
    
    pynode.call('multiply', 3, 4, (err, result) => {
      if (err) {
        reject(err)
        return
      }
      resolve(result)
    })
  })
}

shortRunningFunction()
  .then(result => {
    console.log('short result : ', result)
    parentPort.postMessage(result)
  })
  .catch(e => {
    console.log('short err : ', e)
    parentPort.postMessage({error: e})
  })
