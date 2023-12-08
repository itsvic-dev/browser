project(
  'browser', 'cpp',
  version: '0.1',
  default_options: ['warning_level=3']
)

libcurl = dependency(
  'libcurl',
  required: true, fallback: ['curl'],
  default_options: { 'ssl': 'enabled', },
)

exe = executable(
  'browser', 'browser.cpp',
  install: true,
  dependencies: [libcurl],
)
