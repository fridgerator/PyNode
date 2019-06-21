declare namespace pynode {
  declare function startInterpreter(pythonpath?: string): void;
  declare function appendSysPath(path: string): void;
  declare function openFile(filename: string): void;
  declare function call(...args: any[]): any;
}
