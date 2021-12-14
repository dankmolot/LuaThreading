#ifndef RETRO_LUATHREADING_HPP
#define RETRO_LUATHREADING_HPP

#include <mutex>
#include <condition_variable>
#include <queue>
#include <lua.hpp>

namespace Retro 
{
	namespace LuaThreading
	{
		struct Lock {
			std::mutex m;
			std::condition_variable cv;
			bool step1 = false;
			bool step2 = false;
		};

		class LuaState {
			lua_State* _L = nullptr;
			int _ref = 0;
		public:
			// Default constructor
			LuaState() {};
			LuaState(lua_State* L, int ref) noexcept
				: _L(L)
				, _ref(ref) 
			{}

			// Copy constructor
			LuaState(const LuaState& other) = delete;
			// Copy assignment operator
			LuaState& operator=(const LuaState& other) = delete;

			// Move constructor
			LuaState(LuaState&& other) noexcept
			{
				_L = other._L;
				_ref = other._ref;

				other._L = nullptr;
				other._ref = 0;
			}

			// Move assignment operator
			LuaState& operator=(LuaState&& other) noexcept
			{
				if (this != &other) {
					destroy();

					_L = other._L;
					_ref = other._ref;

					other._L = nullptr;
					other._ref = 0;
				}
				
				return *this;
			}

			// Destroys lua state and returns true if lua state was valid, otherwise false
			bool destroy() 
			{
				if (is_valid()) {
					luaL_unref(_L, LUA_REGISTRYINDEX, _ref);
					return true;
				}

				return false;
			}

			~LuaState()
			{
				destroy();
			}

			inline lua_State* get()
			{
				return _L;
			}

			inline bool is_valid() const
			{
				return _L != nullptr && _ref != LUA_REFNIL && _ref != LUA_NOREF;
			}
		};

		static std::mutex global_mutex;
		static std::queue<Lock> lock_queue;

		LuaState CreateState(lua_State* L)
		{
			lua_State* L2 = lua_newthread(L);
			int ref = luaL_ref(L, LUA_REGISTRYINDEX);

			return { L2, ref };
		}

		Lock* Sync()
		{
			Lock* lock;

			{
				std::lock_guard<std::mutex> lck(global_mutex);
				lock_queue.emplace();
				lock = &lock_queue.back();
			}

			std::unique_lock<std::mutex> lck(lock->m);
			lock->cv.wait(lck, [&lock] { return lock->step1; });

			return lock;
		}

		void Desync(Lock* lock)
		{
			{ // Unlocking main thread
				std::lock_guard<std::mutex> lck(lock->m);
				lock->step1 = true;
			}
			lock->cv.notify_one();
		}

		void Think()
		{
			while (!lock_queue.empty()) {
				Lock& lock = lock_queue.front();

				{ // Unlocking thread
					std::lock_guard<std::mutex> lck(lock.m);
					lock.step1 = true;
				}
				lock.cv.notify_one();

				{ // Waiting thread
					std::unique_lock<std::mutex> lck(lock.m);
					lock.cv.wait(lck, [&lock] { return lock.step2; });
				}

				// Our work done here
				lock_queue.pop();
			}
		}
	}
}

#endif