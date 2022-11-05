
#include <lua.hpp>
#include "lua_threading.hpp"

LuaThreading::LuaState::LuaState(lua_State* L, int ref) noexcept {
	_L = L;
	_ref = ref;
}

LuaThreading::LuaState::LuaState(LuaThreading::LuaState&& other) noexcept {
	_L = other._L;
	_ref = other._ref;

	other._L = nullptr;
	other._ref = 0;
}

LuaThreading::LuaState& LuaThreading::LuaState::operator=(LuaThreading::LuaState&& other) noexcept {
	if (this != &other) {
		auto L = _L;
		auto ref = _ref;

		_L = other._L;
		_ref = other._ref;

		other._L = L;
		other._ref = ref;
	}

	return *this;
}

bool LuaThreading::LuaState::destroy() {
	if (is_valid()) {
		luaL_unref(_L, LUA_REGISTRYINDEX, _ref);
		_L = nullptr;
		_ref = 0;

		return true;
	}

	return false;
}

LuaThreading::LuaState::~LuaState() {
	destroy();
}

bool LuaThreading::LuaState::is_valid() const {
	return _L != nullptr && _ref != LUA_REFNIL && _ref != LUA_NOREF;
}

void LuaThreading::initialize() {
	thread_id = std::this_thread::get_id();
}

int LuaThreading::think(lua_State* L) {
	if (queue.empty()) return 0;

	std::lock_guard<std::mutex> guard(queue_mutex);
	while (!queue.empty()) {
		auto& lock = queue.front();

		{
			std::unique_lock ulock(lock->m);
			lock->step1 = true;

			// Unlocking thread
			lock->cv.notify_one();

			// Waiting for thread
			lock->cv.wait(ulock, [&lock] { return lock->step2; });

			lock->step1 = false;
			lock->step2 = false;
		}

		queue.pop();
	}

	return 0;
}

LuaThreading::LockPointer LuaThreading::get_lock() {
	return std::make_shared<Lock>();
}

void LuaThreading::sync(LuaThreading::LockPointer &lock) {
	if (std::this_thread::get_id() == thread_id)
		return;

	{
		std::lock_guard<std::mutex> guard(queue_mutex);
		queue.push(lock);
	}

	std::unique_lock<std::mutex> ulock(lock->m);
	lock->cv.wait(ulock, [&lock] { return lock->step1; });
}

LuaThreading::LockPointer LuaThreading::sync() {
	auto lock = get_lock();

	sync(lock);

	return lock;
}

void LuaThreading::desync(LockPointer& lock) {
	if (!lock->step1)
		return;

	std::lock_guard<std::mutex> guard(lock->m);
	lock->step2 = true;
	lock->cv.notify_one();
}

LuaThreading::LuaState LuaThreading::create_state(lua_State* L) {
	lua_State* L2 = lua_newthread(L);
	int ref = luaL_ref(L, LUA_REGISTRYINDEX);

	return LuaState(L2, ref);
}