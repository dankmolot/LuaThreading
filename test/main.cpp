#include <GarrysMod/Lua/Interface.h>
#include <lua_threading.hpp>

// Example usage
LUA_FUNCTION(test) {

	// Same thread example
	auto lua = LuaThreading::AcquireLua(); // Or LuaThreading::Lua();
	lua->GetField(GarrysMod::Lua::INDEX_GLOBAL, "debug");
	lua->GetField(-1, "Trace");
	lua->Call(0, 0);
	lua->Pop();

	// Detached thread example
	std::thread([] {
		// Here main thread will be locked until LuaThreading::Lua class is destructed
		auto lua = LuaThreading::AcquireLua(); // Lock this thread, until we will get lua state
		lua->GetField(GarrysMod::Lua::INDEX_GLOBAL, "debug");
		lua->GetField(-1, "Trace");
		lua->Call(0, 0);
		lua->Pop();
		// Here main thread will be unlocked
	}).detach();

	// Joined thread example
	std::thread([] {
		// This example behaves the same way as same thread example
		auto lua = LuaThreading::AcquireLua(false); // Forcing object not to do synchronization with main thread, so we won't stuck
		lua->GetField(GarrysMod::Lua::INDEX_GLOBAL, "debug");
		lua->GetField(-1, "Trace");
		lua->Call(0, 0);
		lua->Pop();
	}).join();

	return 0;
}

GMOD_MODULE_OPEN() {
	LuaThreading::Initialize(LUA);

	LUA->PushCFunction(test);
	LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "test");

	// Registering think
	LUA->GetField(GarrysMod::Lua::INDEX_GLOBAL, "timer");
	LUA->GetField(-1, "Create");
	LUA->PushString("test"); // identifier
	LUA->PushNumber(0); // delay
	LUA->PushNumber(0); // repetitions
	LUA->PushCFunction(LuaThreading::Think);
	LUA->Call(4, 0);
	LUA->Pop();

	return 0;
}

GMOD_MODULE_CLOSE() {
	LuaThreading::Deinitialize(LUA);
	return 0;
}