cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(qMagneB6-plugin SHARED
   src/MagneB6_player.cpp
   src/player_registration.cpp
)

target_include_directories(qMagneB6-plugin PUBLIC include)
target_link_libraries(qMagneB6-plugin qcore)
