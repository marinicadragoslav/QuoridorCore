cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_library(qcore ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
# Create qcore shared library
add_library(qMB6-plugin SHARED
   src/MB6_player.cpp
   src/MB6_board.cpp
   src/MB6_logger.cpp
   src/MB6_timer.cpp
   src/player_registration.cpp
)

target_include_directories(qMB6-plugin PUBLIC include)
target_link_libraries(qMB6-plugin qcore)
