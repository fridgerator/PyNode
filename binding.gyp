{
  "targets": [
    {
      "target_name": "PyNode",
      "sources": [
        "src/main.cpp",
        "src/helpers.cpp",
        "src/pynode.cpp",
        "src/worker.cpp"
      ],
      'include_dirs': [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")"],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
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
            "PY_HOME%": "<!(python -c \"import sysconfig;print(sysconfig.get_paths()['data'])\")"
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
          "include_dirs": [
            "<!(python3-config --includes | sed 's/-I//g' | sed 's/ .*//g')"
          ],
          "ldflags": [
            "<!(python3-config --ldflags)",
          ]
        }]
      ]
    }
  ]
}