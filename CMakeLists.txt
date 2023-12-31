cmake_minimum_required(VERSION 3.12)

project(yacht_test)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin/$<CONFIG>>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/bin/$<CONFIG>>)

if(NOT TARGET yacht)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/yacht.cmake)
endif()

set(LOCAL_TESTING TURE)
if(PROJECT_IS_TOP_LEVEL OR LOCAL_TESTING)
    add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yacht_test)
    target_link_libraries(${PROJECT_NAME} yacht)
endif()
