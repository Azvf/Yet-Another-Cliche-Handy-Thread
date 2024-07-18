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
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    file(GLOB test_files CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test/*.h)
    add_executable(${PROJECT_NAME} ${test_files} ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT yacht_test)
    target_link_libraries(${PROJECT_NAME} yacht gtest_main)
    
    target_include_directories(${PROJECT_NAME} PRIVATE ${googletest_SOURCE_DIR}/googletest/include)
endif()
