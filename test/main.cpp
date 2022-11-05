#include <GarrysMod/Lua/Interface.h>
#include <lua.hpp>
#include <lua_threading.hpp>

static LuaThreading LuaThreadingSingleton;

void register_think(GarrysMod::Lua::ILuaBase* LUA) {
	LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "timer");
	LUA->GetField(-1, "Create");
	LUA->PushString("test"); // identifier
	LUA->PushNumber(0); // delay
	LUA->PushNumber(0); // repetitions
	LUA->PushCFunction([](lua_State* L) { LuaThreadingSingleton.think(); return 0; });
	LUA->Call(4, 0);
	LUA->Pop();

	Msg("Registered think\n");
}

LUA_FUNCTION(test) {
	// Creating new state
	auto state = LuaThreadingSingleton.create_state(LUA->GetState());
	
	// Moving string from one state to another
	std::cout << "Received string: " << LUA->GetString(1) << std::endl;
	lua_pushstring(state.get(), LUA->GetString(1));


	// Creating detached thread
	std::thread([state = std::move(state)] {
		// Syncing main thread with detached
		auto lock = LuaThreadingSingleton.sync();

		// Printing out string
		std::cout << "String in our custom state: " << lua_tostring(state.get(), 1) << std::endl;

		// Desyncing threads
		LuaThreadingSingleton.desync(lock);
	}).detach();

	return 0;
}

GMOD_MODULE_OPEN() {
	LuaThreadingSingleton.initialize();

	register_think(LUA);

	LUA->PushCFunction(test);
	LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "test");

	return 0;
}

GMOD_MODULE_CLOSE() {
	LuaThreadingSingleton.deinitialize();

	return 0;
}