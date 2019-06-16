{
  "targets": [
    {
      "target_name": "main",
      "sources": [
        "main.cc"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "Python-3.7.3",
        "Python-3.7.3/Include"
      ],
      "libraries": [
        # "-L<(module_root_dir)/python3.6/lib",
        # "-L<(module_root_dir)/Python-3.7.3",
        "-L/usr/local/lib",
        "-lpython3.7m"
      ]
    }
  ]
}
