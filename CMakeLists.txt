cmake_minimum_required(VERSION 3.25)

add_library(LuaThreading STATIC 
    src/lua_threading.cpp
    src/lua_threading.hpp
)

target_link_libraries(LuaThreading PUBLIC gmod::common gmod::lua_shared)
