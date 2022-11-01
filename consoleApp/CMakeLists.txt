cmake_minimum_required(VERSION 3.0)

find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create quoridor-cli executable
add_library(consoleApp
   src/ConsoleApp.cpp
)

target_include_directories(consoleApp PUBLIC include)
target_link_libraries(consoleApp qcore)
