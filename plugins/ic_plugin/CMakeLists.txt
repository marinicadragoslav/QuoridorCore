cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create qcore shared library
add_library(ic-plugin SHARED
   src/ICPlayer.cpp
   src/PlayerRegistration.cpp
)

target_compile_options(ic-plugin PRIVATE -Werror -Wall -Wextra)
target_include_directories(ic-plugin PUBLIC include)
target_link_libraries(ic-plugin qcore)
