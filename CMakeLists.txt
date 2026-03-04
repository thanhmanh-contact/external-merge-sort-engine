cmake_minimum_required(VERSION 3.10)
project(ExternalMergeSortEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(sort 
    src/main.cpp 
    src/ExternalMergeSorter.cpp 
    src/ReplacementSelection.cpp 
    src/BinaryFileIO.cpp
)