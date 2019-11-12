const { Worker, isMainThread, parentPort, workerData } = require('worker_threads')

const TIMEOUT = 1000

const threadCount = 1
const threads = new Set()
for (let i = 0; i < threadCount; i++) {
  setTimeout(() => {
    threads.add(
      new Worker(`${process.cwd()}/long_worker.js`)
    )
  }, i  * TIMEOUT)
}

setTimeout(() => {
  console.log('threads : ', threads.size)
  for (let worker of threads) {
    worker.on('message', msg => {
      console.log('long msg : ', msg)
    })
    worker.on('error', err => {
      console.log('long worker err : ', err)
      threads.delete(worker)
      if (threads.size === 0) {
        console.log('done')
        process.exit(0)
      }
    })
    worker.on('exit', () => {
      console.log('long worker done')
      threads.delete(worker)
      if (threads.size === 0) {
        console.log('finished')
        process.exit(0)
      }
    })
  }
}, (threadCount * TIMEOUT) + 100)

let x = 0

setInterval(() => {
  if (x < 5) {
    x++
    let worker = new Worker(`${process.cwd()}/short_worker.js`)
    worker.on('message', msg => {
      console.log('short msg : ', msg)
    })
    worker.on('error', err => {
      console.log('short err : ', err)
    })
    worker.on('exit', () => {
      console.log('short worker done')
    })
  }
  console.log('.')
}, 1000)
