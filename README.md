# md5
Implementation of [RFC 1321](https://www.rfc-editor.org/rfc/rfc1321) the MD5 Message-Digest Algorithm

## Background

I was studying up on hashing algorithms and found that the reference implementation of MD5 in the RFC is ridiculously hard to read / understand. It is written in macro-heavy C code and uses a lot of variable names that don't match the RFC spec description.

My objective was simply to write a more readable reference implementation using modern C++. Efficiency / performance is not the main objective, and there may be some bugs (hopefully not...).

## Setup
I am using [meson](https://mesonbuild.com) to build the project, and [conan](https://docs.conan.io/2/index.html) to manage dependencies (which in this case is just the test framework [Catch2](https://github.com/catchorg/Catch2/tree/devel)). **NOTE: that C++20 is required**.

1. install `meson` and `conan` if needed
1. install dependencies with `conan install . --output-folder=.conan --build=missing`
1. configure project with `meson setup --native-file .conan/conan_meson_native.ini <builddir>`

## Build + Run
Once everything is setup, you can build with `meson compile -C <builddir>`.

When you run the program, it expects a single argument which should be the name of the file whos contents you want to process.

You can also verify results with `md5sum` which is available on most *nix systems.

## Test
You can run the unit tests with `meson test -C <builddir>`. 
