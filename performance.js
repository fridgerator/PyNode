const nodePython = require('./build/Release/PyNode')

const ARRAY_SIZE = 20000000

let largeArray = Array(ARRAY_SIZE).fill().map((_, i) =>
  parseFloat((Math.random() * (0.120 - 0.0200) + 0.0200).toFixed(4)))

let start = new Date()
nodePython.call('return_immediate', largeArray)
console.log('took : ', new Date() - start)
