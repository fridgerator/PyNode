{
  "targets": [
    {
      "target_name": "PyNode",
      "sources": [
        "src/main.cpp",
        "src/helpers.cpp",
        "src/pynode.cpp",
        "src/worker.cpp",
        "src/pywrapper.cpp",
        "src/jswrapper.c"
      ],
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'cflags+': [ '-g' ],
      'cflags_cc+': [ '-g' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "conditions": [
        ['OS=="mac"', {
          'cflags+': ['-fvisibility=hidden'],
          'xcode_settings': {
            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
          }
        }],
        ['OS=="win"', {
          "variables": {
            "PY_HOME%": "<!(if [ -z \"$PY_HOME\" ]; then echo $(python -c \"import sysconfig;print(sysconfig.get_paths()['data'])\"); else echo $PY_HOME; fi)"
          },
          "include_dirs": [
            "<!(echo <(PY_HOME)\include)"
          ],
          "msvs_settings": {
            "VCLinkerTool": {
              "AdditionalLibraryDirectories": "<!(echo <(PY_HOME)\libs)"
            }
          }
        }],
        ['OS!="win"', {
          "variables": {
            "PY_INCLUDE%": "<!(if [ -z \"$PY_INCLUDE\" ]; then echo $(python build_include.py); else echo $PY_INCLUDE; fi)",
            "PY_LIBS%": "<!(if [ -z \"$PY_LIBS\" ]; then echo $(python build_ldflags.py); else echo $PY_LIBS; fi)"
          },
          "include_dirs": [
            "<(PY_INCLUDE)"
          ],
          "libraries": [
            "<(PY_LIBS)",
          ]
        }]
      ]
    }
  ]
}
