libdomrenderer_inc = include_directories('include')

freetype2 = dependency(
  'freetype2',
  required: true,
)

fontconfig = dependency(
  'fontconfig',
  required: true,
)

libdomrenderer_lib = library(
    'components-libdomrenderer',

    'renderer.cpp',
    'viewport.cpp',

    include_directories: [libdomrenderer_inc],
    install: true,
    dependencies: [
        libdom,
        freetype2,
        fontconfig,
    ],
)

libdomrenderer = declare_dependency(
    link_with: libdomrenderer_lib,
    include_directories: libdomrenderer_inc,
    dependencies: [
        libdom,
        freetype2,
        fontconfig,
    ],
)
