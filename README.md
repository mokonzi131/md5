# md5
Implementation of [RFC 1321](https://www.rfc-editor.org/rfc/rfc1321) the MD5 Message-Digest Algorithm

## Background

I was studying up on hashing algorithms and found that the reference implementation of MD5 in the RFC is ridiculously hard to read / understand. It is written in macro-heavy C code and uses a lot of variable names that don't match the RFC spec description.

My objective was simply to write a more readable reference implementation using modern C++. Efficiency / performance is not the main objective, and there may be some bugs (hopefully not...).

## Build + Run
I have included a [meson](https://mesonbuild.com) build file although it should be easy enough to just compile directly if desired. C++ 20 is required.

1. `meson setup <builddir>`
2. `meson compile -C <builddir>`

When you run the program, it expects a single argument which should be the name of the file whos contents you want to process.

You can verify results with `md5sum` which is available on most *nix systems.

