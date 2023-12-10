libhtml_inc = include_directories('include')

libhtml_lib = library(
    'components-libhtml',

    'ast.cpp',
    'tokenizer.cpp',
    'tokens.cpp',
    
    include_directories: [libhtml_inc],
    install: true,
    dependencies: [
        libdom,
    ],
)

libhtml = declare_dependency(
    link_with: libhtml_lib,
    include_directories: libhtml_inc,
    dependencies: [
        libdom,
    ],
)


libhtml_tokenize_test = executable(
    'libhtml_tokenize_test',
    'test/tokenize.cpp',
    dependencies: [libhtml]
)

test_inputs = [
    'example.com.html',
    'fumonet-template.html',
    # 'fumonet.html',
    'nsg650.html',
]

foreach input : test_inputs
    test(
        f'tokenize - @input@', libhtml_tokenize_test,
        workdir: meson.current_source_dir(),
        args: [f'test/@input@'],
    )
endforeach
