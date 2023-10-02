#ifndef LUA_THREADING_HPP
#define LUA_THREADING_HPP

#pragma once

#include <GarrysMod/Lua/LuaBase.h>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>

namespace LuaThreading {
	// Global Thread ID to determine if we need to sync threads
	extern std::thread::id MainThreadID;

	struct SyncLock {
		std::mutex m;
		std::condition_variable cv;
		bool step1 = false;
		bool step2 = false;
	};

	typedef std::shared_ptr<SyncLock> SyncLockPointer;

	class Lua {
	public:
		GarrysMod::Lua::ILuaBase* state = nullptr;

		Lua();
		Lua(bool sync);
		
		// Copy
		Lua(const Lua& other) = delete;
		Lua& operator=(const Lua& other) = delete;

		// Move
		Lua(Lua&& other) noexcept;
		Lua& operator=(Lua&& other) = delete;

		~Lua();

		GarrysMod::Lua::ILuaBase* operator->() const;
        GarrysMod::Lua::ILuaBase* Get() const;
        lua_State* GetState() const;

	private:
		SyncLockPointer lock;

		void ReceiveState(bool sync);
		void SetupState();
	};

	bool NeedSync();

	Lua AcquireLua();
	Lua AcquireLua(bool sync);

	void Initialize(GarrysMod::Lua::ILuaBase* LUA);
	void Deinitialize(GarrysMod::Lua::ILuaBase* LUA);

	int Think(lua_State* L);
	int Think(GarrysMod::Lua::ILuaBase* LUA);
}

#endif // LUA_THREADING_HPP
