#pragma once

#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <atomic>

namespace wws {

	enum class tp_strategy:unsigned int
	{
		over_run_add,
		over_run_invariant
	};

	enum class th_state:unsigned int
	{
		not_alive,
		free,
		busy,
		sleep,
		wait_work
	};


	class m_thread
	{
	public:
		m_thread()
		{
			state = th_state::not_alive;
			running = true;
			
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
		m_thread(m_thread&& t)
		{
			thr = std::move(t.thr);
			state = t.state;
			store_task = std::move(t.store_task);
			running = t.running;
			
			t.state = th_state::not_alive;
			t.running = false;
		}

		m_thread& operator=(const m_thread&) = delete;
		m_thread& operator=(m_thread&& t)
		{
			thr = std::move(t.thr);
			state = t.state;
			store_task = std::move(t.store_task);
			running = t.running;
			
			t.state = th_state::not_alive;
			t.running = false;
		}

		th_state get_state()
		{
			return state;
		}

		bool stop()
		{
			if (!running)
				return;
			if ((bool)store_task)
			{
				return false;
			}else{
				running = false;
			}
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

			state = th_state::not_alive;
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
		thread_pool();
		~thread_pool();
		thread_pool(const thread_pool&) = delete;
		thread_pool(thread_pool&&) = delete;
		thread_pool& operator=(const thread_pool&) = delete;
		thread_pool& operator=(thread_pool&&) = delete;
	private:

	};
}