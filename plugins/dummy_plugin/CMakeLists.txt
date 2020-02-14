cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

# Create qcore shared library
add_library(qdummy-plugin SHARED
   src/DummyPlayer.cpp
   src/PlayerRegistration.cpp
)

target_compile_options(qdummy-plugin PRIVATE -Werror -Wall -Wextra)
target_include_directories(qdummy-plugin PUBLIC include)
target_link_libraries(qdummy-plugin qcore)
