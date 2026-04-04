cmake_minimum_required(VERSION 3.30)
project(scaffold-install-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(scaffold REQUIRED COMPONENTS core csv json convert)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE scaffold::convert)
