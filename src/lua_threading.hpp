#include <GarrysMod/Lua/LuaBase.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <iostream>
#include <queue>

class LuaThreading {
public:
	struct Lock {
		std::mutex m;
		std::condition_variable cv;
		bool step1 = false;
		bool step2 = false;
	};

	typedef std::shared_ptr<Lock> LockPointer;

	class LuaState {
	private:
		lua_State* _L = nullptr;
		int _ref = 0;
	public:
		LuaState() = delete;
		LuaState(lua_State* L, int ref) noexcept;

		// Copy
		LuaState(const LuaState& other) = delete;
		LuaState& operator=(const LuaState& other) = delete;

		// Move
		LuaState(LuaState&& other) noexcept;
		LuaState& operator=(LuaState&& other) noexcept;

		bool destroy();
		~LuaState();

		inline lua_State* get() const noexcept { return _L; }
		inline lua_State* operator*() const noexcept { return get(); }

		bool is_valid() const;
	};

private:
	std::thread::id thread_id;
	std::queue<LockPointer> queue;
	std::mutex queue_mutex;

public:
	void initialize();
	void deinitialize();
	void think();

	LockPointer get_lock();

	void sync(LockPointer &lock);
	LockPointer sync();
	
	void desync(LockPointer &lock);

	LuaState create_state(lua_State* L);
};