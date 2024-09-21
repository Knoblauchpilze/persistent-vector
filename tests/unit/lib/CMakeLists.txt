
target_include_directories(persistent_vector_tests PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)

add_subdirectory(
	${CMAKE_CURRENT_SOURCE_DIR}/storage
	)
