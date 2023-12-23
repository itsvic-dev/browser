project(
  'browser', 'cpp',
  version: '0.1',
  default_options: {
    'warning_level': '3',
    'werror': true,
    'optimization': '2',
    'cpp_std': 'c++17'
  }
)

libcurl = dependency(
  'libcurl',
  required: get_option('curl_shell'), fallback: ['curl'],
  default_options: {
    'ssl': 'enabled',
  },
)

subdir('components/libdom')
subdir('components/libhtml')
subdir('components/libdomrenderer')

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(input: 'include/version.h.in', output: 'version.h', configuration: conf_data)
global_includes = [
  include_directories('.'),
  include_directories('include'),
]

if libcurl.found()
  subdir('shells/curl')
endif
