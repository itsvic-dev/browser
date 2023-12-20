libdoinc = include_directories('include')

libdolib = library(
    'components-libdom',

    'comment.cpp',
    'element.cpp',
    'node.cpp',
    'text.cpp',
    
    include_directories: libdoinc,
    install: true,
)

libdom = declare_dependency(
    link_with: libdolib,
    include_directories: libdoinc,
)
