project(device_listener_test CXX)
find_package(GTest REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.65.1 REQUIRED system)

set(SRC_DIR "../device_listener")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

add_executable(${PROJECT_NAME} main.cpp
                CounterTest.cpp
                TransportTest.cpp
                ${SRC_DIR}/RfcTransport.cpp 
                ${SRC_DIR}/MsgCounter.cpp)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${SRC_DIR}
    ${GTEST_INCLUDE_DIRS}
)

target_link_libraries(
  ${PROJECT_NAME} ${GTEST_LIBRARIES} Threads::Threads ${Boost_LIBRARIES})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_compile_options(${PROJECT_NAME} PRIVATE "-Wall")