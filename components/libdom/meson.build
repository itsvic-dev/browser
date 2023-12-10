libdom_inc = include_directories('include')

libdom_lib = library(
    'components-libdom',

    'comment.cpp',
    'element.cpp',
    'node.cpp',
    'text.cpp',
    
    include_directories: libdom_inc,
    install: true,
)

libdom = declare_dependency(
    link_with: libdom_lib,
    include_directories: libdom_inc,
)
