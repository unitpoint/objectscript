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

add_definitions( -DOS_SQLITE3_DISABLED )
add_definitions( -DOS_REGEXP_DISABLED )

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_custom_target(strip_${target} ALL COMMAND ${CMAKE_STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${target})
endif()

add_executable(${target} ${SOURCES} ${HEADERS})
target_link_libraries(${target} ${CURL_LIBRARIES})
install(TARGETS ${target} RUNTIME DESTINATION bin)
