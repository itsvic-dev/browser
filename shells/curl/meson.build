exe = executable(
  'browser_curl', 'main.cpp',
  install: true,
  dependencies: [
    # external deps
    libcurl,

    # internal deps
    libhtml,
    libdomrenderer,
  ],
  include_directories: [
    global_includes
  ],
)
