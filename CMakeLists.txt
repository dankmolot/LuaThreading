cmake_minimum_required(VERSION 3.25)

add_library(LuaThreading STATIC EXCLUDE_FROM_ALL 
    src/lua_threading.cpp
    src/lua_threading.hpp
)

target_include_directories(LuaThreading PUBLIC src)
target_link_libraries(LuaThreading gmod::common gmod::lua_shared)

if (WIN32)
    set_target_properties(LuaThreading PROPERTIES OUTPUT_NAME "lua_threading.lib") # vs2022 is weirdo
endif()