//call subprocess and wait answer sync
const pynode = require('@fridgerator/pynode');
const {spawnSync} = require('child_process');


libpython_proc = spawnSync('find_libpython');
if(libpython_proc.status != 0){
    // console.log(libpython_proc)
    throw Error(libpython_proc.error);
}

path = libpython_proc.stdout.toString().trim();

pynode.dlOpen(path);

pynode.startInterpreter();

//tell pynode where python script are
pynode.appendSysPath(__dirname);

pynode.openFile('example');

// call the python function and get a return value
pynode.call('test', (err, result) => {
    if (err) return console.log('error : ', err);
    console.log('result : ', result);
});
