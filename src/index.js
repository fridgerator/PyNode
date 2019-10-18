let PyNode = require('../build/Release/PyNode')

// Catch unkown methods, and try to 'call' them
// This allows calling python methods directly on pynode
// like: pynode.some_py_function(1, 2, (err, result) => {})
const proxy = new Proxy(PyNode, {
  get: (receiver, name) => {
    if (receiver[name]) return receiver[name]
    return (...args) => {
      receiver.call(name, ...args)
    }
  }
})

module.exports = proxy
