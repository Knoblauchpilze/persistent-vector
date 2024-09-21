
add_subdirectory(
	${CMAKE_CURRENT_SOURCE_DIR}/lib
	)

target_include_directories(persistent_vector_tests PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)
