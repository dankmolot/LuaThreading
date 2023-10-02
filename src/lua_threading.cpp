#include "lua_threading.hpp"
#include <lua.hpp>
#include <GarrysMod/Lua/Interface.h>

#include <queue>


namespace LuaThreading {
	// Initializing global variables
	std::thread::id MainThreadID;
	static std::mutex GlobalLock;
	static std::queue<SyncLockPointer> Locks;
	static GarrysMod::Lua::ILuaBase* GlobalState = nullptr;
		
	bool NeedSync() {
		return std::this_thread::get_id() != MainThreadID;
	}

	Lua AcquireLua()
	{
		return Lua();
	}

	Lua AcquireLua(bool sync)
	{
		return Lua(sync);
	}

	void Initialize(GarrysMod::Lua::ILuaBase* LUA) {
		MainThreadID = std::this_thread::get_id();
		GlobalState = LUA;
	}

	void Deinitialize(GarrysMod::Lua::ILuaBase* LUA) {
		GlobalState = nullptr;
	}

	int Think(lua_State* L)
	{
		auto LUA = L->luabase;
		LUA->SetState(L);

		return Think(LUA);
	}
	int Think(GarrysMod::Lua::ILuaBase* LUA) {
		if (!Locks.empty()) {
			std::lock_guard<std::mutex> guard(GlobalLock);

			auto& lock = Locks.front();
			{
				std::unique_lock<std::mutex> ulock(lock->m);
				lock->step1 = true;

				// Unlocking thread
				lock->cv.notify_one();

				// Waiting for thread
				lock->cv.wait(ulock, [&lock] { return lock->step2; });

				lock->step1 = false;
				lock->step2 = false;
			}
			Locks.pop();
		}

		return 0;
	}

	// Lua class implementation
	Lua::Lua() {
		ReceiveState(NeedSync());
		SetupState();
	}

	Lua::Lua(bool sync) {
		ReceiveState(sync);
		SetupState();
	}

	Lua::Lua(Lua&& other) noexcept {
		state = other.state;
		LUA = other.LUA;

		lock.swap(other.lock);
	}

	//Lua& Lua::operator=(Lua&& other) noexcept {
	//	// TODO: insert return statement here
	//}

	Lua::~Lua() {		
		// Unlocking if locked
		if (lock && lock->step1) {
			std::lock_guard<std::mutex> guard(lock->m);
			lock->step2 = true;
			lock->cv.notify_one();
		}
	}

	GarrysMod::Lua::ILuaBase* Lua::Get() const {
		return LUA;
	}

	GarrysMod::Lua::ILuaBase* Lua::operator->() const {
		return Get();
	}

	lua_State* Lua::GetState() const {
		return state;
	}

	void Lua::ReceiveState(bool sync) {
		if (!sync) {
			return;
		}

		lock = std::make_shared<SyncLock>();

		GlobalLock.lock();
		Locks.push(lock);
		GlobalLock.unlock();

		std::unique_lock<std::mutex> ulock(lock->m);
		lock->cv.wait(ulock, [&] { return lock->step1; });
		state = LUA->GetState();
	}

	void Lua::SetupState() {
		LUA = GlobalState;
	}
}
