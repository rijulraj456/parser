# public headers and sources
set(PARSER_HEADERS
	characters.h
	charclasses.h
	defs.h
	devhelpers.h
	forest.h
	parser.h
	parser.tmpl.h
	recoders.h
	tgf.h
)
set(PARSER_SOURCES
	characters.cpp
	charclasses.cpp
	parser.cpp
	recoders.cpp
)
set(PARSER_DEFINITIONS
	WITH_DEVHELPERS
	MEASURE
	MEASURE_EACH_POS
)

# object library
################
add_library(${PARSER_OBJECT_LIB_NAME} OBJECT)
target_sources(${PARSER_OBJECT_LIB_NAME} PRIVATE ${PARSER_SOURCES})
target_setup(${PARSER_OBJECT_LIB_NAME})
if(NOT WIN32)
 	target_compile_options(${PARSER_OBJECT_LIB_NAME} PRIVATE -fPIC)
endif()
target_compile_definitions_if(${PARSER_OBJECT_LIB_NAME} PRIVATE "${PARSER_DEFINITIONS}")
target_include_directories(${PARSER_OBJECT_LIB_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
)
if(EMSCRIPTEN_DIR)
	target_compile_definitions(${PARSER_OBJECT_LIB_NAME}
		PRIVATE "-DEMSCRIPTEN_DIR=${EMSCRIPTEN_DIR}"
	)
endif()

# shared library
################
if(BUILD_SHARED_LIBRARY)
	add_library(${PARSER_SHARED_LIB_NAME} SHARED)
	add_library(${namespace}::${PROJECT_SHORT_NAME} ALIAS ${PARSER_SHARED_LIB_NAME})
	target_sources(${PARSER_SHARED_LIB_NAME} PRIVATE ${PARSER_SOURCES})
	target_setup(${PARSER_SHARED_LIB_NAME})
	target_compile_definitions_if(${PARSER_SHARED_LIB_NAME} PRIVATE "${PARSER_DEFINITIONS}")
	target_include_directories(${PARSER_SHARED_LIB_NAME} PUBLIC
		$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
		$<INSTALL_INTERFACE:.>
	)
	set_target_properties(${PARSER_SHARED_LIB_NAME} PROPERTIES
		EXPORT_NAME ${PROJECT_NAME}
		PUBLIC_HEADER "${PARSER_HEADERS}"
	)
endif()

# static library
################
add_library(${PARSER_STATIC_LIB_NAME} STATIC)
target_sources(${PARSER_STATIC_LIB_NAME} PRIVATE ${PARSER_SOURCES})
target_setup(${PARSER_STATIC_LIB_NAME})
target_compile_definitions_if(${PARSER_STATIC_LIB_NAME} PRIVATE "${PARSER_DEFINITIONS}")
target_include_directories(${PARSER_STATIC_LIB_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
)
set_target_properties(${PARSER_STATIC_LIB_NAME} PROPERTIES
	OUTPUT_NAME ${PROJECT_NAME}
	PUBLIC_HEADER "${PARSER_HEADERS}"
)
if(NOT BUILD_STATIC_LIBRARY)
	exclude(${PARSER_STATIC_LIB_NAME})
endif()

# install
set(BUILD_TARGETS ${PARSER_OBJECT_LIB_NAME})
if(BUILD_SHARED_LIBRARY)
	set(BUILD_TARGETS "${BUILD_TARGETS}" ${PARSER_SHARED_LIB_NAME})
endif()
if(BUILD_STATIC_LIBRARY)
	set(BUILD_TARGETS "${BUILD_TARGETS}" ${PARSER_STATIC_LIB_NAME})
endif()
include(installation)
