cmake_minimum_required(VERSION 3.1)
project(liby262dec)

find_package(Threads)

set(Y262DEC_TARGET_ARCH "unknown")
if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
	set(Y262DEC_TARGET_ARCH "intelx86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|x86.*|i386.*")
	set(Y262DEC_TARGET_ARCH "intelx86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64.*|aarch64.*|ARM64.*|AARCH64.*)")
	set(Y262DEC_TARGET_ARCH "arm64")
endif()

set(liby262dec_sources_assembly "")

if(Y262DEC_TARGET_ARCH MATCHES "intelx86")
	set(liby262dec_sources_assembly
		${CMAKE_CURRENT_SOURCE_DIR}/transform_x86.h
		${CMAKE_CURRENT_SOURCE_DIR}/transform_x86.c
		${CMAKE_CURRENT_SOURCE_DIR}/mc_x86.h
		${CMAKE_CURRENT_SOURCE_DIR}/mc_x86.c
	)
elseif(Y262DEC_TARGET_ARCH MATCHES "arm64")
	set(liby262dec_sources_assembly
		${CMAKE_CURRENT_SOURCE_DIR}/transform_arm64.c
		${CMAKE_CURRENT_SOURCE_DIR}/transform_arm64.h
		${CMAKE_CURRENT_SOURCE_DIR}/mc_arm64.h
		${CMAKE_CURRENT_SOURCE_DIR}/mc_arm64.c
	)
endif()



add_library(liby262dec STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/y262dec.h
	${CMAKE_CURRENT_SOURCE_DIR}/y262dec.c
	${CMAKE_CURRENT_SOURCE_DIR}/api/y262decapi.h
	${CMAKE_CURRENT_SOURCE_DIR}/y262decapi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bitstream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bitstream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/aboveslicelevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/aboveslicelevel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/slicelevel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/slicelevel.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tables.c
    ${CMAKE_CURRENT_SOURCE_DIR}/threads.h
    ${CMAKE_CURRENT_SOURCE_DIR}/threads.c
    ${CMAKE_CURRENT_SOURCE_DIR}/transform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/transform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/mc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mc.c
	
	${liby262dec_sources_assembly}
)

set_target_properties(liby262dec PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(liby262dec PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/api)

if(Y262DEC_TARGET_ARCH MATCHES "intelx86")
	target_compile_definitions(liby262dec PRIVATE ASSEMBLY_X86)
elseif(Y262DEC_TARGET_ARCH MATCHES "arm64")
	target_compile_definitions(liby262dec PRIVATE ASSEMBLY_ARM64)
endif()

if(WIN32)
	# always has threads
else()
	target_compile_definitions(liby262dec PRIVATE HAVE_LIBPTHREAD)
endif()

target_link_libraries(liby262dec PUBLIC Threads::Threads)