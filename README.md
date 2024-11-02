# <img src="docs/media/logo.svg" alt="Gemstore" width="400" />

Gemstore is an in-development key-value data store.

# Authors:
 * Anamitra Ghorui
 * Sachin Mudaliyar

# Building

## Prerequisites

Your system must have GCC 11, Clang 14, or any other equivalent compiler that
supports C++11.

Your system must also have CMake (>= 3.22) installed.

## Instructions

Create a build folder and go inside of it:

```
mkdir build
cd build
```

Run CMake

```
cmake ..
```

Once successful, run `make`

```
make
```

This will create two main executables: `client` and `server`. Executables of
testing programs (`src/tests/*.cpp`) will also be compiled, and put in a
subdirectory called `tests` in the build folder.

## Generating `compile_commands.json`

To generate metadata for `clangd`, you may use a tool like `bear` to generate
a `compile_commands.json` file. After the Run Cmake step above, you may do:

```
bear -- make
```

This will both build the program and generate the metadata in the project build
folder. This needs to be done only once, and can be re-ran if in case the
CMakeLists.txt is modified and/or when the source structure is changed.

You may also use `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`. This is already specified
by default in the build file.

# Attributions

This project makes significant use of these other open source projects in its
C++ components. These are included directly in the source tree in the
`remote_deps` folder. The licenses for each are present in `remote_deps/licenses`.

* **[nlohmann/json](https://github.com/nlohmann/json)**: Modern header-only C++
  JSON library. [License (MIT)](./remote_deps/licenses/LICENSE_nlohmann_json)

* **[yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)**:
  A C++ header-only HTTP/HTTPS server and client library.
  [License (MIT)](./remote_deps/licenses/LICENSE_yhirose_httplib)

