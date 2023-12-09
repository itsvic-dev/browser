project(
  'browser', 'cpp',
  version: '0.1',
  default_options: {
    'warning_level': 3,
    'werror': true,
    'optimization': 2,
    'cpp_std': 'c++17'
  }
)

libcurl = dependency(
  'libcurl',
  required: true, fallback: ['curl'],
  default_options: {
    'ssl': 'enabled',
  },
)

subdir('components/libhtml')

exe = executable(
  'browser', 'browser.cpp',
  install: true,
  dependencies: [
    # external deps
    libcurl,

    # internal deps
    libhtml,
  ],
)
