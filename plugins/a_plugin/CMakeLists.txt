cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create qcore shared library
add_library(a-plugin SHARED
   src/APlugin.cpp
   src/PlayerRegistration.cpp
   src/Board.cpp
   src/TurnGenerator.cpp
)

target_compile_options(a-plugin PRIVATE -Werror -Wall -Wextra)
target_include_directories(a-plugin PUBLIC include)
target_link_libraries(a-plugin qcore)
