
add_executable (persistent_vector)

add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	)

target_sources (persistent_vector PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/main.cc
	)

target_include_directories (persistent_vector PUBLIC
	)

target_link_libraries(persistent_vector
	persistent_vector_lib
	)
