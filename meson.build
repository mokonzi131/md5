project(
    'MD5 Hash Generator',
    'cpp',
    default_options: ['warning_level=3', 'cpp_std=c++20'],
    license: 'MIT',
    # license_files: 'LICENSE', ... needs Meson 1.1.0
    meson_version: '>=1.0.1',
    version: '0.1.0')

# main executable
incdir = include_directories('inc')
sources = [
    'src/blockloader.cpp', 
    'src/md5.cpp',
]
executable(
    'md5', 
    sources: sources + ['src/main.cpp'], 
    include_directories: incdir)
