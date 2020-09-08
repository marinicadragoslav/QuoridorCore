cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(a-plugin SHARED
   src/APlugin.cpp
   src/PlayerRegistration.cpp
   src/Board.cpp
   src/TurnGenerator.cpp
)

target_include_directories(a-plugin PUBLIC include)
target_link_libraries(a-plugin qcore)
