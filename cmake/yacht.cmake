file(GLOB yacht_sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
add_library(yacht INTERFACE ${yacht_sources})
target_include_directories(yacht INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)