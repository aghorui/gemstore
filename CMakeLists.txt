# gemstore - CMake Build File

cmake_minimum_required(VERSION 3.22)

set(MINIMUM_GCC_VERSION 11.0)

project(gemstore)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(REMOTE_DEP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/remote_deps/include")
set(TEST_OUTPUT_DIR "${CMAKE_BINARY_DIR}/tests")

# Add include directory for headers.
include_directories("${SRC_ROOT}/include")
include_directories("${REMOTE_DEP_DIR}")

add_compile_options("-std=c++11" "-Wall" "-Wextra" "-Wno-unused-function" "-g")

file(GLOB TEST_SOURCES "${SRC_ROOT}/tests/*.cpp")

# The source dependencies. May be compiled to a library if needed.
set(LIBSRC
	"${SRC_ROOT}/networking.cpp"
	"${SRC_ROOT}/store.cpp"
	"${SRC_ROOT}/common.cpp"
)

add_library(gem ${LIBSRC})


# Compile the client and the server to an executable
add_executable("server" "${SRC_ROOT}/server.cpp")
target_link_libraries("server" gem)

add_executable("client" "${SRC_ROOT}/client.cpp")
target_link_libraries("client" gem)


# Compile every single source file in the test directory to an executable
foreach(TEST_SOURCE ${TEST_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable("${TEST_NAME}" ${TEST_SOURCE})
	target_link_libraries("${TEST_NAME}" gem)
	set_target_properties(${TEST_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${TEST_OUTPUT_DIR}")
endforeach()