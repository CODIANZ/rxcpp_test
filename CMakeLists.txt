cmake_minimum_required(VERSION 3.0.0)
project(rxcpp_test VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(rxcpp_test main.cpp)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include_directories(./rxcpp/Rx/v2/src)
