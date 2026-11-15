# Installation

## Quick Start

### Install using vcpkg or Conan

You can install the latest release of sqlgen
using either [vcpkg](https://vcpkg.io/en/package/sqlgen) or [Conan](https://conan.io/center/recipes/sqlgen).

### Build using vcpkg

Alternatively, you can build sqlgen from source using vcpkg:

1. Set up vcpkg:
```bash
git submodule update --init
./vcpkg/bootstrap-vcpkg.sh  # Linux, macOS
./vcpkg/bootstrap-vcpkg.bat # Windows
```

2. Build the library:
```bash
cmake -S . -B build -DCMAKE_CXX_STANDARD=20 -DCMAKE_BUILD_TYPE=Release
cmake --build build -j 4  # gcc, clang
cmake --build build --config Release -j 4  # MSVC
```

This will build the static library. To build the shared library
add `-DBUILD_SHARED_LIBS=ON -DVCPKG_TARGET_TRIPLET=...` to the first line.
Run `./vcpkg/vcpkg help triplets` to view all supported triplets.
Common triplets for shared libraries are `x64-linux-dynamic`,
`arm64-osx-dynamic` or `x64-osx-dynamic`.

Add `-DSQLGEN_POSTGRES=ON` to the first line to support PostgreSQL. Add `-DSQLGEN_MYSQL=ON` to support MySQL/MariaDB. Add `-DSQLGEN_DUCKDB=ON` to support DuckDB.

On Linux and macOS, you may need to install additional dependencies to get Postgres to compile:

```bash
sudo apt-get install autoconf bison flex # Linux (Ubuntu, Debian, ...)
brew install autoconf bison flex # macOS
```

If you want to build the tests, add `-DSQLGEN_BUILD_TESTS=ON`.
If you want to build 'dry' tests only (those that do not require a database connection), add `-DSQLGEN_BUILD_DRY_TESTS_ONLY=ON`.

3. Include in your CMake project:
```cmake
find_package(sqlgen REQUIRED)
target_link_libraries(your_target PRIVATE sqlgen::sqlgen)
```

### Build using Conan

You can also build sqlgen from source using Conan:

1. Install Conan (assuming you have Python and pipx installed):

```bash
pipx install conan
conan profile detect
```

For older versions of pip, you can also use `pip` instead of `pipx`.

2. Build the library:

```bash
conan build . --build=missing -s compiler.cppstd=gnu20
```

This will build the static library. To build the shared library,
add `-o */*:shared=True`.

Add `-o sqlgen/*:with_mysql=True` to support MySQL/MariaDB.

3. Include in your CMake project:
```cmake
find_package(sqlgen REQUIRED)
target_link_libraries(your_target PRIVATE sqlgen::sqlgen)
```

You can call `conan inspect .` to get an overview of the supported options.

