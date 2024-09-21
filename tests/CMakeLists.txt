
find_package (GTest REQUIRED)

add_executable (persistent_vector_tests)

add_subdirectory(
	${CMAKE_CURRENT_SOURCE_DIR}/unit
	)

target_sources (persistent_vector_tests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/main.cc
	)

target_include_directories (persistent_vector_tests PUBLIC
	${GTEST_INCLUDE_DIRS}
	)

target_link_libraries(persistent_vector_tests
persistent_vector_lib
	${GTEST_LIBRARIES}
	)
