const { Worker, isMainThread, parentPort, workerData } = require('worker_threads')

const TIMEOUT = 1000

const threadCount = 3
const threads = new Set()
for (let i = 0; i < threadCount; i++) {
  setTimeout(() => {
    threads.add(
      new Worker(`${process.cwd()}/worker.js`)
    )
  }, i  * TIMEOUT)
}

setTimeout(() => {
  console.log('threads : ', threads.size)
  for (let worker of threads) {
    worker.on('message', msg => {
      console.log('msg : ', msg)
    })
    worker.on('error', err => {
      console.log('worker err : ', err)
      threads.delete(worker)
      if (threads.size === 0) {
        console.log('done')
        process.exit(0)
      }
    })
    worker.on('exit', () => {
      console.log('worker done')
      threads.delete(worker)
      if (threads.size === 0) {
        console.log('done')
        process.exit(0)
      }
    })
  }
}, (threadCount * TIMEOUT) + 100)

setInterval(() => {
  console.log('.')
}, 1000)
