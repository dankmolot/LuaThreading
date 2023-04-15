cmake_minimum_required(VERSION 3.20)

project(LuaThreading VERSION 1.0.0 LANGUAGES CXX)

add_library(LuaThreading STATIC EXCLUDE_FROM_ALL 
    src/lua_threading.cpp
    src/lua_threading.hpp
)

target_include_directories(LuaThreading PUBLIC src)
target_link_libraries(LuaThreading gmod::common gmod::lua_shared)
