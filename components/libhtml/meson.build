libhtml_inc = include_directories('include')

libhtml_lib = library(
    'components-libhtml',

    'exceptions.cpp',
    'parser.cpp',
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


libhtml_canParse_test = executable(
    'libhtml_canParse_test',
    'test/canParse.cpp',
    dependencies: [libhtml]
)

test_inputs = [
    'basic.html',
    'eofInText.html',
    'headInHead.html',
    'invalidDoctype.html',
    # 'missingDoctype.html',  # segfaults
    'noHtmlTag.html',
    # 'noscriptInHead.html',  # unknown insertion mode encountered: in head noscript
    # 'scriptInHead.html',  # unknown tokenizer state encountered: 4
    'spacesBeforeDoctype.html',
    # 'styleTag.html',  # segfaults
    'textBeforeDoctype.html',
    # 'weirdEndTags.html',  # segfaults
]

foreach input : test_inputs
    test(
        f'can parse - @input@', libhtml_canParse_test,
        workdir: meson.current_source_dir(),
        args: [f'test/cases/@input@'],
    )
endforeach
