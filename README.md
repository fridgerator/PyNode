# PyNode

[![Build Status](https://travis-ci.org/fridgerator/PyNode.svg?branch=master)](https://travis-ci.org/fridgerator/PyNode)
	
### Call python code from node.js

**Only tested with python 3.6.x and python 3.7.x.  Probably wont work with Python 2.7**

## Installation

All operating systems require some environment variables be present so that PyNode can find your local python installation.  These will be different depending on how Python was installed on your system and will vary even between the same OS.

* Set `PYTHON_INCLUDE_PATH` to your python includes path (contains both `Python.h` and `pyconfig.h` )
* Set `PYTHON_LIB_PATH` to the path that contains the python lib
* Set `PYTHON_LIB` to the python lib name (mac and linux only)

On Ubuntu, it might look something like:
```bash
export PYTHON_INCLUDE_PATH=/usr/local/include/python3.7m
export PYTHON_LIB_PATH=/usr/local/lib
export PYTHON_LIB=python3.7m
```

Mac using the .pkg installer:
```bash
export PYTHON_INCLUDE_PATH=/Library/Frameworks/Python.framework/Versions/3.7/include/python3.7m
export PYTHON_LIB_PATH=/Library/Frameworks/Python.framework/Versions/3.7/lib
export PYTHON_LIB=python3.7m
```

And on Windows installed via official installer for the local user:
```bash
set PYTHON_INCLUDE_PATH="C:\Users\<username>\AppData\Local\Programs\Python\Python3.6\include"
set PYTHON_LIB_PATH="C:\Users\<username>\AppData\Local\Programs\Python\Python3.6\libs"
```

## Usage
