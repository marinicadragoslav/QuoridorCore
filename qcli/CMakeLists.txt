cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create quoridor-cli executable
add_executable(quoridor-cli
   src/main.cpp
   src/ConsoleApp.cpp
   src/ConsolePlayer.cpp
)

target_compile_options(quoridor-cli PRIVATE -Werror -Wall -Wextra)
target_include_directories(quoridor-cli PRIVATE include)
target_link_libraries(quoridor-cli qcore)

