# beman.free_value_or: Free Function constrained `value_or`

<!--
SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
-->

<!-- markdownlint-disable line-length -->
[![Library Status](https://raw.githubusercontent.com/bemanproject/beman/refs/heads/main/images/badges/beman_badge-beman_library_under_development.svg)](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#the-beman-library-maturity-model)
[![Continuous Integration Tests](https://github.com/bemanproject/free_value_or/actions/workflows/ci_tests.yml/badge.svg)](https://github.com/bemanproject/free_value_or/actions/workflows/ci_tests.yml)
[![Lint Check (pre-commit)](https://github.com/bemanproject/free_value_or/actions/workflows/pre-commit-check.yml/badge.svg)](https://github.com/bemanproject/free_value_or/actions/workflows/pre-commit-check.yml)
[![Coverage](https://coveralls.io/repos/github/bemanproject/free_value_or/badge.svg?branch=main)](https://coveralls.io/github/bemanproject/free_value_or?branch=main)
![Standard Target](https://github.com/bemanproject/beman/blob/main/images/badges/cpp29.svg)
<!-- markdownlint-restore -->

`beman.free_value_or` provides free function alternatives to member `value_or`
operations for nullable types such as `std::optional`, `std::expected`, and
pointers to objects.

**Implements**: Free `value_or`, `reference_or`, `or_invoke`, and `or_construct`
operations proposed by this project.

**Status**: [Under development and not yet ready for production use.](https://github.com/bemanproject/beman/blob/main/docs/beman_library_maturity_model.md#under-development-and-not-yet-ready-for-production-use)

## License

`beman.free_value_or` is licensed under the Apache License v2.0 with LLVM Exceptions.

## Usage

Include the public umbrella header and call the functions from
`smd::free_value_or`.

Full runnable examples can be found in [`examples/`](examples/).

## Dependencies

### Build Environment

This project requires at least the following to build:

* A C++ compiler that conforms to the C++20 standard or greater, and that provides
  either `std::reference_constructs_from_temporary_v` or the
  `__reference_constructs_from_temporary` builtin — GCC 13, Clang 18, or MSVC and
  later. The dangling-reference rejection in `reference_or` is a static_assert built
  on that trait, so a compiler without it cannot build the library.
* CMake 3.30 or later
* (Test Only) Catch2

You can disable building tests by setting CMake option `BEMAN_FREE_VALUE_OR_BUILD_TESTS` to
`OFF` when configuring the project.

### Supported Platforms

| Compiler   | Version | C++ Standards | Standard Library  |
|------------|---------|---------------|-------------------|
| GCC        | 16-13   | C++26-C++17   | libstdc++         |
| Clang      | 22-19   | C++26-C++17   | libstdc++, libc++ |
| Clang      | 18      | C++26-C++17   | libc++            |
| Clang      | 18      | C++23-C++17   | libstdc++         |
| AppleClang | latest  | C++26-C++17   | libc++            |
| MSVC       | latest  | C++23         | MSVC STL          |

GCC 12 and earlier lack the `__reference_constructs_from_temporary` builtin. Clang 17
is excluded by the vendored `beman::optional` test suite, which needs range adaptors
(`std::views::join`) that libc++ 17 does not provide; nothing in `free_value_or` itself
requires it.

The MSVC row is C++23 only, and the `msvc-debug` / `msvc-release` presets configure at
C++23 rather than inheriting the C++20 default the other presets use. Below C++23 the
MSVC CRT still declares the pre-C++17 `::unexpected()` terminate handler in `<eh.h>`,
which is ambiguous with `beman::expected::unexpected` throughout the vendored
`beman::expected` test suite. `free_value_or`'s own headers are C++20-clean on MSVC.

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
