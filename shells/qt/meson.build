qt6 = import('qt6')

moc_files = qt6.compile_moc(
  headers: ['mainwindow.h', 'renderview.h'],
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

exe = executable(
  'browser_qt',

  'main.cpp',
  'mainwindow.cpp',
  'renderview.cpp',
  moc_files,

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
