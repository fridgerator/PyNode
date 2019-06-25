# PyNode
	
### Call python code from node.js

<span style="color:red">Only tested with python 3.6.x and python 3.7.x.  Probably wont work with Python 2.7</span>

## Installation

All operating systems require some environment variables be present so that PyNode can find your local python installation.

* Set `PYTHON_INCLUDE_PATH` to your python includes path (contains both `Python.h` and `pyconfig.h` )
* Set `PYTHON_LIB_PATH` to the path that contains the python lib
* Set `PYTHON_LIB` to the python lib name

On Ubuntu, looks something like:
```bash
export PYTHON_INCLUDE_PATH=/usr/local/include/python3.7m
export PYTHON_LIB_PATH=/usr/local/lib
export PYTHON_LIB=python3.7m
```

And on Windows:
```bash
set 
```

## Usage
