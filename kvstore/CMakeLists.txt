cmake_minimum_required(VERSION 3.22)

set(MINIMUM_GCC_VERSION 11.0)

project(kvstore)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories("include")

add_compile_options("-std=c++11" "-Wall" "-Wextra" "-Wno-unused-function" "-g")

set(LIBSRC
	"./buffer/bufferPool.cc"
  "./disk/diskManager.cc"
)

add_library(kv ${LIBSRC})

# Compile the client and the server to an executable
add_executable("main" "./main.cc")
target_link_libraries("main" kv)

