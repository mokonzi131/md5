project(
    'MD5 Hash Generator',
    'cpp',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
    license: 'MIT',
    # license_files: 'LICENSE', ... needs Meson 1.1.0
    meson_version: '>=1.0.1',
    version: '0.1.0')

incdir = include_directories('inc')
sources = [
    'src/blockloader.cpp', 
    'src/main.cpp'
]
executable('md5', sources: sources, include_directories: incdir)
