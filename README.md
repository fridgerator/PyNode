# PyNode

[![Build Status](https://travis-ci.org/fridgerator/PyNode.svg?branch=master)](https://travis-ci.org/fridgerator/PyNode)
	
### Call python code from node.js

**Node v10 or above is required**

**Tested with python 3.6 - 3.8.**

## Installation

**BEFORE NPM INSTALL OR YARN INSTALL**

* Make sure `python` in your system `PATH` is the correct one: `python --version`. You may use a virtualenv to do this.
* Install gyp-next: `git clone https://github.com/nodejs/gyp-next`; `cd gyp-next`; `python setup.py install`
* `yarn add @fridgerator/pynode` or
`npm install @fridgerator/pynode`
* If your default `python` is version 2.7, then you may have to yarn install using additional env variables: `PY_INCLUDE=$(python3.6 build_include.py) PY_LIBS=$(python3.6 build_ldflags.py) yarn`

## Usage

### Async API

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

### Full Object API

The Full Object API allows full interaction between JavaScript objects within Python code, and Python objects within JavaScript code. In Python, this works very transparently without needing to know if a value is a Python or JS object. In JavaScript, the interface is (currently) a bit more primitive, but supports getting object members and calling objects using `.get()` and `.call()`.

Primitives are generally converted between JS and Python values, so passing strings, numbers, lists, and dicts generally works as expected. The wrappers described above only kick in for non-primitives.

In a python file `test_files/objects.py`:

```python

def does_things_with_js(jsobject):
    '''This function demonstrates that we can treat passed-in
    JavaScript objects (almost) as if they were native Python objects.
    '''
    # We can print out JS objects; toString is called automatically
    # (it is mapped to Python's __str__):
    print("does_things_with_js:", jsobject)
    
    # We can call any function on the JS object. Objects passed
    # in are converted to JS if they are primitives, or wrapped in the
    # Full Object API if not, so JavaScript can also call back into Python:
    result = jsobject.arbitraryFunction("nostrongtypinghere")
    
    # The result from JavaScript is also converted back into Python:
    print(result)
    
    # The result will be converted back into JavaScript by PyNode:
    return result

class PythonClass(object):
    '''A simple class to demonstrate creating and calling an instance from JavaScript.'''

    def __init__(self, a, b):
        '''Constructor. a and b are converted to Python types by PyNode'''
        self.a = a
        self.b = b

    def collate(self, callback):
        '''A function that is given a callback to call into. This can be a JavaScript function'''
        print('PythonClass.collate()')
        print('callback:', callback)
	
	# Arguments are converted into JavaScript objects, and the return value
	# from the callback is converted back into Python objects.
        return callback(self.a, self.b, "hello", 4)

    def __repr__(self):
        return 'PythonClass(a=%r, b=%r)' % (self.a, self.b)

```

In Node:

```javascript
const pynode = require('@fridgerator/pynode')

pynode.startInterpreter();
pynode.appendSysPath('./test_files/');

/* Modules are imported as JS objects, implementing the Full Object API */
const objectsmodule = pynode.import('objects'); /* test module in test_files */
const python_builtins = pynode.import('builtins'); /* access to python builtins such as str, all, etc, if you want them. */

/* Define an example JavaScript class: */
class JSClass {
    constructor(item) {
        this.item = item
    }
    arbitraryFunction(value) {
        console.log("arbitraryFunction called; item = " + this.item);
        console.log("                         value = " + value);
        return value + 12;
    }
    toString() {
        return "JSClass{item=" + this.item + "}"
    }
}

/* Create an instance of JSClass which Python will have access to: */
jsclassinstance = new JSClass(['some', 'data']);
console.log(jsclassinstance);

/* Get the 'does_things_with_js' Python function from the objects module,
   and call it, passing in the JSClass object: */
result = objectsmodule.get('does_things_with_js').call(jsclassinstance);

/* Python objects are automatically converted back to JS, or returned as
   PyNodeWrappedPythonObject instances (see below( if they are
   non-primitive types:
*/
console.log(result);

/* Create a Python class. In Python this is done by calling the class
   object with constructor arguments (ie, don't use `new`): */
somedict = {'some': 'dict'};
pyclassinstance = objectsmodule.get('PythonClass').call(somedict, '2');

/* The resulting object is returned as a PyNodeWrappedPythonObject instance,
   which implements the Full Object API: */
console.log(pyclassinstance);

/* The PyNodeWrappedPythonObject instance can have members retrieved from it
   and functions called. Below is the equivalent of (in Python) 
   `getattr(pyclassinstance, 'collate').__call__(<some javascript function>)`
   or put simply:
   `pyclassinstance.collate(<some javascript function>)`.
   The JavaScript function is also converted to a callable Python object */
*/
result = pyclassinstance.get('collate').call(function(a, b, c, d) {
    console.log(arguments);
    return c * d; /* JavaScript return values are converted back to Python types by PyNode */
});
console.log(result);
```
