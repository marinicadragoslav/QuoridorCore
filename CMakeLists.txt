cmake_minimum_required(VERSION 3.0)

project(quoridor VERSION 1.0.0 LANGUAGES CXX)

# Set the output folder where your program will be created
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/export/bin)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(qcore)
add_subdirectory(qcli)
add_subdirectory(plugins)