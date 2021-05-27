cmake_minimum_required(VERSION 3.1)
project(y262decapp)

set( SRC_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/main.c
)

add_executable(y262decapp ${SRC_FILES})

target_link_libraries(y262decapp liby262dec)

set_target_properties(y262decapp PROPERTIES 
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
	POSITION_INDEPENDENT_CODE ON
)
