cmake_minimum_required(VERSION 2.8)

set(target oscript)
project(${target})

list(APPEND SOURCES os.cpp)
list(APPEND SOURCES ./../../source/objectscript.cpp)
list(APPEND SOURCES ./../../source/ext-curl/oscurl.cpp)

list(APPEND HEADERS ./../../source/ext-curl/oscurl.h)

find_package(CURL QUIET)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
else()
    message(FATAL_ERROR "[ ERROR ]: Could not find cURL")
endif()

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_custom_target(strip_${target} ALL COMMAND ${CMAKE_STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${target})
endif()

add_executable(${target} ${SOURCES} ${HEADERS})
target_link_libraries(${target} ${CURL_LIBRARIES})
install(TARGETS ${target} RUNTIME DESTINATION bin)
