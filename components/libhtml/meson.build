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


libhtml_tokenize_test = executable(
    'libhtml_tokenize_test',
    'test/tokenize.cpp',
    dependencies: [libhtml]
)

test('tokenize - example.com', libhtml_tokenize_test, workdir: meson.current_source_dir(),
    args: ['test/example.com.html'])
test('tokenize - fumonet-template', libhtml_tokenize_test, workdir: meson.current_source_dir(),
    args: ['test/fumonet-template.html'])
# currently fails because of HTML entities
# test('tokenize - fumonet', libhtml_tokenize_test, workdir: meson.current_source_dir(),
#     args: ['test/fumonet.html'])
test('tokenize - nsg650', libhtml_tokenize_test, workdir: meson.current_source_dir(),
    args: ['test/nsg650.html'])
