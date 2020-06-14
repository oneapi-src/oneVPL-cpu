# oneAPI Video Processing Library

The oneAPI Video Processing Library (oneVPL) provides a single video processing
API for encode, decode, and video processing that works across a wide range of
accelerators.

It comes with bindings for C, C++, and Python.

## Building

### Requirements

- A C/C++ Compiler
  - GCC on Linux
  - Microsoft Visual Studio, MinGW on Windows
- [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)
- [yasm](https://yasm.tortall.net/)
- [CMake](https://cmake.org/)
- [Python3](https://www.python.org/)
- [git](https://git-scm.com/)
- [Doxygen](http://www.doxygen.nl/) (to build documentation)
- [Ninja-build](https://ninja-build.org/), [Meson](https://mesonbuild.com/) (to build AV1 decoder - libav1d)

### Canonical build

Ubuntu bash shell:
```
script/bootstrap
script/build
```

Windows cmd prompt:
```
script\bootstrap
script\build
```


See the [developer reference](doc/developer-reference/build.md) for more details
on configuring the build.

### Building documentation

```
mkdir _build
cd _build
cmake ..
cmake --build . --target doc
```

### Build output

You can find the resulting output in `_build` or `_build\Release`.

### Other builds

See the [scripts README](script/README.md) for other build options.

## Contributing

Please see the [CONTRIBUTING.md](CONTRIBUTING.md) file for details.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file
for details.
