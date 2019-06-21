{
  "targets": [
    {
      "target_name": "PyNode",
      "sources": [
        "src/main.cpp",
        "src/helpers.cpp"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        # "Python-3.7.3",
        # "Python-3.7.3/Include",
        "/Library/Frameworks/Python.framework/Versions/3.6/include/python3.6m"
      ],
      "libraries": [
        # "-L/usr/local/lib",
        # "-lpython3.7m"
        "-L/Library/Frameworks/Python.framework/Versions/3.6/lib",
        "-lpython3.6m"
      ]
    }
  ]
}
