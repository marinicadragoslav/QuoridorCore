cmake_minimum_required(VERSION 3.0)

# Set dependencies
find_package(Threads REQUIRED)

# Create qcore shared library
add_library(qcore SHARED
   src/GameController.cpp
   src/Game.cpp
   src/BoardState.cpp
   src/Player.cpp
   src/PlayerAction.cpp
   src/PluginManager.cpp
)

target_compile_options(qcore PRIVATE -Werror -Wall -Wextra)
target_include_directories(qcore PUBLIC include)
target_link_libraries(qcore Threads::Threads stdc++fs ${CMAKE_DL_LIBS})
