cmake_minimum_required(VERSION 3.10)
project(unixNet)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
# Set the output directory for the libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
# Set the output directory for the archives
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
# Set the output directory for the object files
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_OBJECT_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmake/${CMAKE_BUILD_TYPE})
# Set the output directory for the cmake files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(datetime_client src/datetime_client.cpp)
add_executable(datetime_server src/datetime_server.cpp)
add_executable(echo_client src/echo_client.cpp)
add_executable(echo_server src/echo_server.cpp)
add_executable(concurrent_echo_server src/concurrent_echo_server.cpp)

