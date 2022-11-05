# LuaThreading

This module was created for fixing problem with multi threading in Lua. 
It supports creating dedicated lua state, and syncing main lua thread with another thread.

## How to use
Initialize LuaThreading
```cpp
#include <lua_threading.hpp>
static LuaThreading Threading;

GMOD_MODULE_OPEN() {
	Threading.initialize();

    // ...

	return 0;
}

GMOD_MODULE_CLOSE() {
	Threading.deinitialize();

    // ...

	return 0;
}
```

Create lua state
```cpp
LuaThreading::LuaState state = Threading.create_state(LUA->GetState());

lua_pushstring(*state, "String 1");
lua_pushstring(state.get(), "String 2");
```

Registering think function for thread syncing
```cpp
LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "timer");
LUA->GetField(-1, "Create");
LUA->PushString("my_super_think"); // identifier
LUA->PushNumber(0); // delay
LUA->PushNumber(0); // repetitions
LUA->PushCFunction([](lua_State* L) { Threading.think(); return 0; });
LUA->Call(4, 0);
LUA->Pop();
```

Using lua in different thread
```cpp
std::thread([state = std::move(state)] {
    auto lock = Threading.sync()

    lua_pushstring(*state, "String 3!")

    // Don't forget to desync!
    Threading.desync(lock)
}).detach(); // Don't use .sync() with thread.join()
```