# PyNode

[![Build Status](https://travis-ci.org/fridgerator/PyNode.svg?branch=master)](https://travis-ci.org/fridgerator/PyNode)
	
### Call python code from node.js

**Node v10 or above is required**

**Only tested with python 3.6.x and python 3.7.x.  Probably wont work with Python 2.7**

## Installation

**BEFORE NPM INSTALL OR YARN INSTALL**

* Make sure `python` in your system `PATH` is the correct one: `python --version`
* **Linux and Mac only**: make sure `python-config` is also in your path and is from the correct python installation.  You may have to symlink `python3-config` to `/usr/local/bin`.
* `yarn add @fridgerator/pynode` or
`npm install @fridgerator/pynode`

## Usage

In a python file `test.py`:

```python
def add(a, b):
  return a + b
```
in node:

```javascript
const pynode = require('@fridgerator/pynode')

// Workaround for linking issue in linux:
// https://bugs.python.org/issue4434
// if you get: `undefined symbol: PyExc_ValueError` or `undefined symbol: PyExc_SystemError`
pynode.dlOpen('libpython3.6m.so') // your libpython shared library

// optionally pass a path to use as Python module search path
pynode.startInterpreter()

// add current path as Python module search path, so it finds our test.py
pynode.appendSysPath('./')

// open the python file (module)
pynode.openFile('test')

// call the python function and get a return value
pynode.call('add', 1, 2, (err, result) => {
  if (err) return console.log('error : ', err)
  result === 3 // true
})
```
