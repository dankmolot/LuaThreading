#include "lua_threading.hpp"
#include <lua.hpp>
#include <GarrysMod/Lua/Interface.h>

#include <queue>

struct LuaState {
	lua_State* L = nullptr;
	int ref = 0;
};

namespace LuaThreading {
	// Initializing global variables
	std::thread::id MainThreadID;
	static std::mutex GlobalLock;
	static std::queue<SyncLockPointer> Locks;
	static LuaState GlobalState;
		
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

		GlobalState.L = lua_newthread(LUA->GetState());
		GlobalState.ref = LUA->ReferenceCreate();
	}

	void Deinitialize(GarrysMod::Lua::ILuaBase* LUA) {
		if (GlobalState.ref) {
			LUA->ReferenceFree(GlobalState.ref);

			GlobalState.L = nullptr;
			GlobalState.ref = 0;
		}
	}

	int LuaThreading::Think(lua_State* L)
	{
		auto LUA = L->luabase;
		LUA->SetState(L);

		return Think(LUA);
	}
	int LuaThreading::Think(GarrysMod::Lua::ILuaBase* LUA) {
		if (!Locks.empty()) {
			std::lock_guard<std::mutex> guard(GlobalLock);

			auto& lock = Locks.front();
			{
				std::unique_lock ulock(lock->m);
				lock->step1 = true;
				lock->state = LUA->GetState();

				// Unlocking thread
				lock->cv.notify_one();

				// Waiting for thread
				lock->cv.wait(ulock, [&lock] { return lock->step2; });

				lock->step1 = false;
				lock->step2 = false;
				lock->state = nullptr;
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
		other.state = nullptr;

		lock.swap(other.lock);
	}

	//Lua& Lua::operator=(Lua&& other) noexcept {
	//	// TODO: insert return statement here
	//}

	Lua::~Lua() {
		if (state) {
			state->luabase->SetState(orig_state);
		}
		

		// Unlocking if locked
		if (lock && lock->step1) {
			std::lock_guard<std::mutex> guard(lock->m);
			lock->step2 = true;
			lock->cv.notify_one();
		}
	}

	GarrysMod::Lua::ILuaBase* Lua::operator->() const {
		return state->luabase;
	}

	void Lua::ReceiveState(bool sync) {
		if (!sync) {
			state = GlobalState.L;
			return;
		}

		lock = std::make_shared<SyncLock>();

		GlobalLock.lock();
		Locks.push(lock);
		GlobalLock.unlock();

		std::unique_lock<std::mutex> ulock(lock->m);
		lock->cv.wait(ulock, [&] { return lock->step1; });

		state = lock->state;
	}

	void Lua::SetupState() {
		orig_state = state->luabase->GetState();
		state->luabase->SetState(state);
	}
}