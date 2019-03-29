project(device_listener)
cmake_minimum_required(VERSION 2.8)

add_subdirectory(device_listener)
add_subdirectory(dsimulator)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()