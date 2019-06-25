{
  "targets": [
    {
      "target_name": "PyNode",
      "sources": [
        "src/main.cpp",
        "src/helpers.cpp"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ['OS=="win"', {
          "include_dirs": [
            "<!(echo %PYTHON_INCLUDE_PATH%)"
          ],
          "libraries": [
            "-L<!(echo $PYTHON_LIB_PATH%)",
            "-l<!(echo %PYTHON_LIB%)"
          ]
        }],
        ['OS!="win"', {
          "include_dirs": [
            "<!(echo $PYTHON_INCLUDE_PATH)"
          ],
          "libraries": [
            "-L<!(echo $PYTHON_LIB_PATH)",
            "-l<!(echo $PYTHON_LIB)"
          ]
        }]
      ]
    }
  ]
}
