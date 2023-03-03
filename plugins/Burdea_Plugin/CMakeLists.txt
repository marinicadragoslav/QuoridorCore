cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(burdea-plugin SHARED
   src/BPlayer.cpp
   src/PlayerRegistration.cpp
)

target_include_directories(burdea-plugin PUBLIC include)
target_link_libraries(burdea-plugin qcore)