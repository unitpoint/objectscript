cmake_minimum_required(VERSION 2.8)

set(target oscript)
project(${target})

list(APPEND SOURCES os.cpp)
list(APPEND SOURCES ./../../source/objectscript.cpp)

if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  add_custom_target(strip_${target} ALL COMMAND ${CMAKE_STRIP} ${CMAKE_CURRENT_BINARY_DIR}/${target})
endif()

add_executable(${target} ${SOURCES} ${HEADERS})
install(TARGETS ${target} RUNTIME DESTINATION bin)
