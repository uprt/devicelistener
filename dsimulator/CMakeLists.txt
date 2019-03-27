project(dsimulator C)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
file(GLOB SRC_LIST *.c)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_compile_options(${PROJECT_NAME} PRIVATE -pedantic -Wall -Wextra -Werror)