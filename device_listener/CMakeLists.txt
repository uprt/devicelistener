project(device_listener CXX)
cmake_minimum_required(VERSION 2.8)

find_package(Boost COMPONENTS system)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
file(GLOB SRC_LIST *.cpp)

add_executable(device_listener ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE ./ ${Boost_INCLUDE_DIRS})
target_link_libraries(device_listener Boost::system)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_compile_options(${PROJECT_NAME} PRIVATE -pedantic -Wall -Wextra -Werror)
