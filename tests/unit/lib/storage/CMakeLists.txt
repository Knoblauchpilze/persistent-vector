
target_sources(persistent_vector_tests PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/PersistentVectorTest.cc
	)

target_include_directories(persistent_vector_tests PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	)
