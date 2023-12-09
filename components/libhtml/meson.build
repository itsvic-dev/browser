libhtml_inc = include_directories('include')

libhtml_lib = library(
    'components-libhtml',

    'ast.cpp',
    'tokenizer.cpp',
    'tokens.cpp',
    
    include_directories: libhtml_inc,
)

libhtml = declare_dependency(
    link_with: libhtml_lib,
    include_directories: libhtml_inc,
)
