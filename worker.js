const { parentPort } = require('worker_threads')
const pynode = require('./build/Release/PyNode')

const longRunningFunction = () => {
  return new Promise((resolve, reject) => {
    console.log('cwd : ', process.cwd())

    pynode.startInterpreter()
    pynode.appendSysPath('./test_files')
    pynode.openFile('performance')
    
    pynode.call('generate_slow_number', 5, 7, (err, result) => {
      if (err) {
        reject(err)
        return
      }
      resolve(result)
    })
  })
}

longRunningFunction()
  .then(result => {
    console.log('result : ', result)
    parentPort.postMessage(result)
  })
  .catch(e => {
    console.log('err : ', e)
    parentPort.postMessage({error: e})
  })
