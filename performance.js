const nodePython = require('./build/Release/PyNode')
nodePython.dlOpen('libpython3.6m.so')
nodePython.startInterpreter()
nodePython.appendSysPath('./test_files')
nodePython.openFile("tools")

const ARRAY_SIZE = 2000000

let largeArray = Array(ARRAY_SIZE).fill().map((_, i) =>
  parseFloat((Math.random() * (0.120 - 0.0200) + 0.0200).toFixed(4)))

let start = new Date()
nodePython.call('return_immediate', largeArray)
console.log(`passing array of ${ARRAY_SIZE} points took : `, new Date() - start)

nodePython.openFile("performance")
start = new Date()
nodePython.call('generate_slow_number', 12)
console.log('generate_slow_number took : ', new Date() - start)
nodePython.stopInterpreter()
// nodePython = null
delete nodePython

const np2 = require('./build/Release/PyNode')
console.log('after require')
np2.dlOpen('libpython3.6m.so')
console.log('afterdlopen')
np2.startInterpreter()
console.log('after start interpreter')
np2.appendSysPath('./test_files')
console.log('after append sys path')
np2.openFile("tools")
console.log('after open file')
start = new Date()
np2.call('generate_slow_number', 12)
console.log('generate_slow_number took : ', new Date() - start)

const np3 = require('./build/Release/PyNode')
np3.dlOpen('libpython3.6m.so')
np3.startInterpreter()
np3.appendSysPath('./test_files')
np3.openFile("tools")
start = new Date()
np3.call('generate_slow_number', 12)
console.log('generate_slow_number took : ', new Date() - start)
