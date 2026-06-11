TODO

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater
* CMake 3.30 or later
* (Test Only) GoogleTest

You can disable building tests by setting CMake option `BEMAN_FREE_VALUE_OR_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 16-13   | C++26-C++17   | libstdc++         |
| GCC        | 12-11   | C++23-C++17   | libstdc++         |
| Clang      | 22-19   | C++26-C++17   | libstdc++, libc++ |
| Clang      | 18      | C++26-C++17   | libc++            |
| Clang      | 18      | C++23-C++17   | libstdc++         |
| Clang      | 17      | C++26-C++17   | libc++            |
| Clang      | 17      | C++20, C++17  | libstdc++         |
| AppleClang | latest  | C++26-C++17   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

## Development

See the [Contributing Guidelines](CONTRIBUTING.md).

## Integrate beman.free_value_or into your project

### Build

You can build free_value_or using a CMake workflow preset:

```bash
cmake --workflow --preset gcc-release
```

To list available workflow presets, you can invoke:

```bash
cmake --list-presets=workflow
```

For details on building beman.free_value_or without using a CMake preset, refer to the
[Contributing Guidelines](CONTRIBUTING.md).

### Installation

#### Vcpkg

The preferred way to install free_value_or is via vcpkg. To do so, after installing vcpkg
itself, you need to add support for the Beman project's [vcpkg
registry](https://github.com/bemanproject/vcpkg-registry) by configuring a
`vcpkg-configuration.json` file (which free_value_or [provides](vcpkg-configuration.json)).

Then, simply run `vcpkg install beman-free-value-or`.

#### Manual

To install beman.free_value_or globally after building with the `gcc-release` preset, you can
run:

```bash
sudo cmake --install build/gcc-release
```

Alternatively, to install to a prefix, for example `/opt/beman`, you can run:

```bash
sudo cmake --install build/gcc-release --prefix /opt/beman
```

This will generate the following directory structure:

```txt
/opt/beman
├── include
│   └── beman
│       └── free_value_or
│           ├── free_value_or.hpp
│           └── ...
└── lib
    └── cmake
        └── beman.free_value_or
            ├── beman.free_value_or-config-version.cmake
            ├── beman.free_value_or-config.cmake
            └── beman.free_value_or-targets.cmake
```

### CMake Configuration

If you installed beman.free_value_or to a prefix, you can specify that prefix to your CMake
project using `CMAKE_PREFIX_PATH`; for example, `-DCMAKE_PREFIX_PATH=/opt/beman`.

You need to bring in the `beman.free_value_or` package to define the `beman::free_value_or` CMake
target:

```cmake
find_package(beman.free_value_or REQUIRED)
```

You will then need to add `beman::free_value_or` to the link libraries of any libraries or
executables that include `beman.free_value_or` headers.

```cmake
target_link_libraries(yourlib PUBLIC beman::free_value_or)
```

### Using beman.free_value_or

To use `beman.free_value_or` in your C++ project,
include an appropriate `beman.free_value_or` header from your source code.

```c++
#include <beman/free_value_or/free_value_or.hpp>
```

> [!NOTE]
>
> `beman.free_value_or` headers are to be included with the `beman/free_value_or/` prefix.
> Altering include search paths to spell the include target another way (e.g.
> `#include <free_value_or.hpp>`) is unsupported.
