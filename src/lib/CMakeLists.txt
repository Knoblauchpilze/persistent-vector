
add_library (persistent_vector_lib SHARED "")

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

target_sources (persistent_vector_lib PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/PersistentVector.cc
	${CMAKE_CURRENT_SOURCE_DIR}/PersistentVectorBlock.cc
	)

target_link_libraries (persistent_vector_lib
	)

target_include_directories (persistent_vector_lib PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	)
