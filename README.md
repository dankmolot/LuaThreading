# LuaThreading

This module was created for fixing problem with multi threading in Lua. 
It supports creating dedicated lua state, and syncing main lua thread with another thread.

## How to use
Include LuaThreading into your project
```lua
CreateProject(...)
    -- ...
    IncludeLuaShared() -- Do not forget to include LuaShared
    IncludeLuaThreading()
    -- ...
```

Initialize LuaThreading
```cpp
#include <lua_threading.hpp>

GMOD_MODULE_OPEN() {
    LuaThreading::Initialize(LUA);

    // Register think somewhere here
    // ...

    return 0;
}

GMOD_MODULE_CLOSE() {
	LuaThreading::Deinitialize(LUA);

    // ...

	return 0;
}
```

Registering think function for thread syncing
```cpp
LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "timer");
LUA->GetField(-1, "Create");
LUA->PushString("my_super_think"); // identifier
LUA->PushNumber(0); // delay
LUA->PushNumber(0); // repetitions
LUA->PushCFunction(LuaThreading::Think);
LUA->Call(4, 0);
LUA->Pop();

// Or if you already have think function then just call think

LuaThreading::Think(LUA);
```

Acquiring lua state from anywhere
```cpp
{
    // Getting lua state (and syncing threads if needed)
    LuaThreading::Lua LUA = LuaThreading::AcquireLua(); // Or LuaThreading::Lua();
    // Using Lua as Garrysmod::Lua::ILuaBase*
    LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "print");
    LUA->PushString("Hello World");
    LUA->Call(1, 0);
} // Here main thread will be desynced, if it was synced
  // That means what LuaThreading::Lua will hold main thread until it destroyed


// Or we can force sync by passing boolean
LuaThreading::AcquireLua(true); // Will sync with main thread
LuaThreading::AcquireLua(false); // Won't sync
```

## Reference
```cpp
// lua_threading.hpp

namespace LuaThreading {
    // Global Thread ID to determine if we need to sync threads
    extern std::thread::id MainThreadID;

    class Lua {
    public:
        lua_State* state;

        Lua(); // Runs LuaThreading::NeedSync internally
        Lua(bool sync); // true if you need to sync with main thread

        ~Lua(); // Destructor will unlock main thread, if it was locked

        // Copying is prohibited
        Lua(const Lua& other) = delete;
        Lua& operator=(const Lua& other) = delete;

        // Move constructor is permitted, but not move statement
        Lua(Lua&& other) noexcept;
        Lua& operator=(Lua&& other) = delete;

        // You can use LuaThreading::Lua class as it was Garrysmod::Lua::ILuaBase*
        GarrysMod::Lua::ILuaBase* operator->() const;
    }

    // Returns true, if called in non main thread
    bool NeedSync();

    // Same as LuaThreading::Lua();
    Lua AcquireLua();
    Lua AcquireLua(bool sync);

    void Initialize(GarrysMod::Lua::ILuaBase* LUA);
    void Deinitialize(GarrysMod::Lua::ILuaBase* LUA);

    int Think(lua_State* L);
    int Think(GarrysMod::Lua::ILuaBase* LUA);
}
```

## ToDo
- [ ] Create joinable std::thread with context, so we can avoid deadlocks if thread was joined
