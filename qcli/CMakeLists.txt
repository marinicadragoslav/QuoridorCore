cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
find_library(consoleApp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create quoridor-cli executable
add_executable(quoridor-cli
   src/main.cpp
   src/ConsolePlayer.cpp
)

target_include_directories(quoridor-cli PRIVATE include)
target_link_libraries(quoridor-cli qcore consoleApp)

