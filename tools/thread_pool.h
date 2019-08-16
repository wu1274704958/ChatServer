#pragma once

#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>
#include <queue>
#include <vector>

namespace wws {

	enum class tp_strategy:unsigned int
	{
		//over_run_add,
		over_run_invariant
	};

	enum class th_state:unsigned int
	{
		not_alive,
		free,
		busy,
		sleep,
		wait_die
	};


	class m_thread
	{
	public:
		m_thread()
		{
			state = th_state::not_alive;
			running = false;
			
		}
		m_thread(std::function<void()> f)
		{
			state = th_state::not_alive;
			running = true;

			store_task.swap(f);

			thr = std::thread(&m_thread::thread_body,this);
		}
		~m_thread()
		{
			
		}
		m_thread(const m_thread&) = delete;
		m_thread(m_thread&& t) = delete;
		m_thread& operator=(const m_thread&) = delete;
		m_thread& operator=(m_thread&& t) = delete;

		bool run(std::function<void()> f)
		{
			if (state == th_state::not_alive && running == false)
			{
				running = true;
				store_task.swap(f);
				thr = std::thread(&m_thread::thread_body, this);
				return true;
			}
			return false;
		}

		th_state get_state()
		{
			return state;
		}

		bool stop()
		{
			if (!running)
				return true;
			if ((bool)store_task)
			{
				return false;
			}else{
				running = false;
				return true;
			}
		}

		bool has_stored_task()
		{
			std::lock_guard guard(task_mutex);
			return ((bool)store_task);
		}

		void wait_and_stop()
		{
			while (has_stored_task()) {}
			while (!stop()){}
			if (state == th_state::not_alive && running == false)//not launch
				return;
			while (state != th_state::wait_die){}
			thr.detach();
		}
		
		bool can_set_task()
		{
			if (state == th_state::wait_die)
				return false;
			if (state == th_state::not_alive && running == false)
				return false;
			task_mutex.lock();
			bool stored = (bool)store_task;
			task_mutex.unlock();
			if (stored) return false;
			
			if (state == th_state::not_alive ||
				state == th_state::free ||
				state == th_state::sleep)
			{
				return true;
			}
			return false;
		}

		bool set_task(std::function<void()> f)
		{
			if (!(bool)f)return false;
			task_mutex.lock();
			bool stored = (bool)store_task;
			task_mutex.unlock();
			if (stored)
			{
				return false;
			}
			if (state == th_state::not_alive ||
				state == th_state::free ||
				state == th_state::sleep)
			{
				task_mutex.lock();
				store_task.swap(f);
				task_mutex.unlock();

				return true;
			}
			return false;
		}

		void thread_body()
		{
			state = th_state::free;
			while (running)
			{
				std::function<void()> temp_f;
				task_mutex.lock();
				if ((bool)store_task){
					state = th_state::busy;
					temp_f.swap(store_task);
				}
				task_mutex.unlock();

				if ((bool)temp_f)
				{
					temp_f();
					state = th_state::free;
				}else{
					state = th_state::sleep;
					std::this_thread::yield();
					state = th_state::free;
				}
			}

			state = th_state::wait_die;
		}

	private:
		std::atomic<th_state> state;
		std::thread thr;
		std::function<void()> store_task;
		std::atomic<bool> running;
		std::mutex task_mutex;
	};



	class thread_pool
	{
	public:
		thread_pool(uint32_t max_count)
		{
			this->max_count = max_count;
			strategy = tp_strategy::over_run_invariant;
		}
		thread_pool(tp_strategy strategy,uint32_t max_count)
		{
			this->max_count = max_count;
			this->strategy = strategy;
		}
		void add_task(std::function<void()> f)
		{
			bool can_direct_set_task = false;

			if (!dispatcher_launched)
			{
				auto ptr = find_can_set_task();
				if (ptr == nullptr)
				{
					if (thrs.size() < max_count)
					{
						thrs.push_back(new m_thread(f));
						can_direct_set_task = true;
					}
				}
				else {
					if (ptr->set_task(f))
					{
						can_direct_set_task = true;
					}
					else if(thrs.size() < max_count) {
						thrs.push_back(new m_thread(f));
						can_direct_set_task = true;
					}
				}
			}

			if (!can_direct_set_task){

				mutex_queue.lock();
				task_queue.push(f);
				mutex_queue.unlock();

				if (dispatcher_launched == false)
				{
					dispatcher_launched = true;
					dispatcher.run([this]() {
						this->dispatch_body();
					});
				}
			}
		}

		void dispatch_body()
		{
			while (dispatcher_launched)
			{
				bool has_task = false;
				{
					std::lock_guard guard(mutex_queue);
					has_task = !task_queue.empty();
				}
				if(has_task)
				{
					std::lock_guard queue_guard(mutex_queue);
					std::lock_guard thrs_guard(mutex_thrs);

					auto ptr = find_can_set_task();
					if(ptr){
						if(ptr->set_task(task_queue.front()))
							task_queue.pop();
					}
					else {
						std::this_thread::yield();
					}
				}
				else {
					std::this_thread::yield();
				}
			}
		}

		bool has_not_dispatched()
		{
			if (dispatcher_launched)
			{
				std::lock_guard queue_guard(mutex_queue);
				return !task_queue.empty();
			}
			else {
				return false;
			}
		}

		void wait_all()
		{
			dispatcher_launched = false;
			dispatcher.wait_and_stop();
			while (!thrs.empty())
			{
				m_thread * ptr = nullptr;
				ptr = thrs.back();
				thrs.pop_back();

				ptr->wait_and_stop();
				if (ptr)
					delete ptr;
			}
		}

		m_thread* find_can_set_task()
		{
			for (auto th : thrs)
			{
				if (th->can_set_task())
					return th;
			}
			return nullptr;
		}
		~thread_pool()
		{
			assert(thrs.empty());//must be call wait_all() before destruction
		}
		thread_pool(const thread_pool&) = delete;
		thread_pool(thread_pool&&) = delete;
		thread_pool& operator=(const thread_pool&) = delete;
		thread_pool& operator=(thread_pool&&) = delete;
	private:
		tp_strategy strategy;
		uint32_t max_count;
		std::mutex mutex_thrs;
		std::mutex mutex_queue;
		m_thread dispatcher;
		std::queue<std::function<void()>> task_queue;
		std::vector<m_thread*> thrs;
		std::atomic<bool> dispatcher_launched = false;
	};
}