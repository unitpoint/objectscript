cmake_minimum_required (VERSION 2.8)
project (MPFDParser)



add_library (SharedTarget SHARED Parser.cpp Field.cpp Exception.cpp)
add_library (StaticTarget STATIC Parser.cpp Field.cpp Exception.cpp)


set_target_properties(SharedTarget PROPERTIES OUTPUT_NAME MPFDParser-1)
set_target_properties(StaticTarget PROPERTIES OUTPUT_NAME MPFDParser-1)

INSTALL(TARGETS SharedTarget DESTINATION lib)
INSTALL(TARGETS StaticTarget DESTINATION lib)

INSTALL(FILES Field.h Exception.h Parser.h DESTINATION include/MPFDParser-1)

