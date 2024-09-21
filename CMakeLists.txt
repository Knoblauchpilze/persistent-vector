cmake_minimum_required (VERSION 3.7)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_compile_options (-Wall -Wextra -Werror)

project (persistent-vector LANGUAGES CXX)

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

set (CMAKE_VERBOSE_MAKEFILE OFF)

add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/src
	)

enable_testing ()

add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/tests
	)
