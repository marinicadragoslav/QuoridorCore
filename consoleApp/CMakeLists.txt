cmake_minimum_required(VERSION 3.0)

# Create quoridor-cli executable
add_library(consoleApp
   src/ConsoleApp.cpp
)

target_include_directories(consoleApp PUBLIC include)
