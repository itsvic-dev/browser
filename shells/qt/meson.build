qt6 = import('qt6')

exe = executable(
  'browser_qt',

  'main.cpp',
  'mainwindow.cpp',

  install: true,
  dependencies: [
    # external deps
    qt6_dep,

    # internal deps
    libhtml,
    libdomrenderer,
  ],
  include_directories: [
    global_includes
  ],
)
