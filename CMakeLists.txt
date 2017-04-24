cmake_minimum_required(VERSION 3.5.1)
project(Archiver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES
    Archiver/Archiver.cpp
    Archiver/Archiver.h
    Archiver/FS.h)

add_executable(Archiver ${SOURCE_FILES})