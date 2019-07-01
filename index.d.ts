declare module '@fridgerator/pynode' {
  export function startInterpreter(pythonpath?: string): void;
  export function appendSysPath(path: string): void;
  export function openFile(filename: string): void;
  export function call(...args: any[]): any;
  export function eval(...args: any[]): number;
}
