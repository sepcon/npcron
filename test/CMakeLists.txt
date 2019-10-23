cmake_minimum_required(VERSION 3.5)

project(npcron-test LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${NPCRON_LIBRARY_HEADER_DIR})

add_executable(npcron-test main.cpp)
target_link_libraries(npcron-test npcron)