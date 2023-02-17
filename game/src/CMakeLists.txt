cmake_minimum_required(VERSION 3.5)
project(mochiko_platformer)

set(CMAKE_CXX_STANDARD 14)

set(DPLATFORM Web)

set(PLATFORM Web)

include_directories(/usr/include)

link_directories(/usr/local/lib)

add_executable(mochiko_platformer tile\ editor.cpp)

target_link_libraries(mochiko_platformer raylib) 
