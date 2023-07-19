# md5
Implementation of [RFC 1321](https://www.rfc-editor.org/rfc/rfc1321) the MD5 Message-Digest Algorithm

## Background

I was studying up on hashing algorithms and found that the reference implementation of MD5 in the RFC is ridiculously hard to read / understand. It is written in macro-heavy C code and uses a lot of variable names that don't match the RFC spec description.

My objective was simply to write a more readable reference implementation using modern C++. I am not trying to make it highly efficient and there are probably some bugs.

## Build + Run
Simply put the data you want to hash into the data.txt file and then compile and run main.cpp. It should print out the correct md5 hash.

You can validate the result with **md5sum** on *nix machines.
