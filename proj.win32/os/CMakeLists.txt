cmake_minimum_required(VERSION 2.8)

set(target oscript)
project(${target})

list(APPEND SOURCES os.cpp)
list(APPEND SOURCES ./../../source/objectscript.cpp)

find_package(CURL QUIET)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
	list(APPEND SOURCES ./../../source/ext-curl/os-curl.cpp)
	list(APPEND HEADERS ./../../source/ext-curl/os-curl.h)
else()
    message(STATUS "[ WARNING ]: Could not find cURL")
	add_definitions( -DOS_CURL_DISABLED )
endif()

# begin sqlite3
list(APPEND SOURCES ./../../source/ext-sqlite3/sqlite3.c)
list(APPEND HEADERS ./../../source/ext-sqlite3/sqlite3.h)
list(APPEND SOURCES ./../../source/ext-sqlite3/os-sqlite3.cpp)
list(APPEND HEADERS ./../../source/ext-sqlite3/os-sqlite3.h)

SET(GCC_COVERAGE_COMPILE_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS} ")
SET(GCC_COVERAGE_LINK_FLAGS    "${GCC_COVERAGE_LINK_FLAGS} -lpthread -ldl")
# end sqlite3

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )

# add_definitions( -DOS_SQLITE3_DISABLED )
add_definitions( -DOS_REGEXP_DISABLED )

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_custom_target(strip_${target} ALL COMMAND ${CMAKE_STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${target})
endif()

add_executable(${target} ${SOURCES} ${HEADERS})
target_link_libraries(${target} ${CURL_LIBRARIES} ${PCRE_LIBRARIES})
install(TARGETS ${target} RUNTIME DESTINATION bin)
