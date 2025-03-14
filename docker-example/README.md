#Dockerizing

Dockerizing a pynode application is easy.

This example uses one of  nikolaik/python-nodejs image.
nikolaik's image repository has all combinations of  python and nodejs versions so thats a good start.
You can use a python base image and compile a node
or
use a base nodejs image and compile python

using python3.8 and node 18
```
FROM nikolaik/python-nodejs:python3.8-nodejs18-slim

WORKDIR /usr/app
```

We need to install additional platform packages in order to build pynode app thus it uses node-addons we should able to compile the module
So we are installing required packages as following

```
RUN apt update -qq && apt install make gcc g++ -y
```

It is not required but making sure that pynode is compiling we explicitly run install command for pynode npm package 
```
COPY package.json .
RUN npm install @fridgerator/pynode
RUN npm install 
```

It is bespractice to work with virtual environment for python applications. 
Once your development has finished, pip freeze > requirements.txt will produces dependency file.
Then copy this file and install the requirements in docker image

```
COPY requirements.txt .
RUN pip3 install -r requirements.txt
```

Now image is ready to execute a pynode application.  
You can checkout Dockerfile for details and customize according to your needs.

## Tips

There are multiple python versions depending on the platform and python version, the place of the shared python library could vary and can results problems in the runtime. To prevent this, pynode has dlOpen option to meet required libpython to the application.

If you cant sure where is the python lib, you can use find_libpython python package to locate.
It automatically finds the proper library location according to version and platform.

It can be used as follows to introduce libpython to the pynode application. You can checkout index.js for detailed example.

```
const {spawnSync} = require('child_process');
libpython_proc = spawnSync('find_libpython');
if(libpython_proc.status != 0){
    throw Error(libpython_proc.error);
}

path = libpython_proc.stdout.toString().trim();

pynode.dlOpen(path);
```






