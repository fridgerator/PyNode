declare module '@fridgerator/pynode' {
  /**
   * Fixes dyanmic linking issue in python
   * @param dlFile Python shared library file name
   */
  export function dlOpen(dlFile: string): void;

  /**
   * Initialize the Python interpreter
   *
   * @param pythonpath Optionally set the python module search paths
   */
  export function startInterpreter(pythonpath?: string): void;

  /**
   * Stops the Python interpreter
   */
  // export function stopInterpreter(pythonpath?: string): void;

  /**
   * Add an additional path as a python module search path
   *
   * @param path Path to append to python `sys.path`
   */
  export function appendSysPath(path: string): void;

  /**
   * Open (import) python file as the main PyNode module.
   * Only one file can be open at a time
   *
   * @param filename Python file to open, leave off .py extension
   */
  export function openFile(filename: string): void;

  /**
   * Call a function from the opened python module
   *
   * @param functionName Name of function to call
   * @param args Arguments to python function
   *
   * @example
   * // in python file test.py
   * def add(a, b):
   *   return a + b
   *
   * const pynode = require('@fridgerator/pynode')
   * pynode.startInterpreter()
   * pynode.openFile('test')
   * let x = pynode.call('add', 1, 2)
   * x === 3 // true
   */
  export function call(functionName: string, ...args: any[]): void;
  namespace call {
    function __promisify__(functionName: string, ...args: any[]): Promise<any>;
  }

  /**
   * Pass a string directly to the python interpreter.
   * Returns 0 if the call was successful, -1 if there was an exception.
   * @param statement String statement to be evaluated
   */
  export function eval(statement: string): number;
}
