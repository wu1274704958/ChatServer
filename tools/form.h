#pragma once
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <serialization.hpp>
#include <make_color.hpp>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace wws {

	class LoadFormExce : public std::exception
	{
	public:
		LoadFormExce() : exception("Failed to load form file!") {}
	};

	class SaveFormExce : public std::exception
	{
	public:
		SaveFormExce() : exception("Failed to save form file!") {}
	};

	template <class T>											
	using has_seilza_form_line_t = decltype(std::declval<T>().seilza_form_line(std::declval<std::string&>()));

	template <typename T>										
	using has_seilza_form_line_vt = wws::is_detected<has_seilza_form_line_t, T>;

	template <class T>
	using has_seilza_to_line_t = decltype(std::declval<T>().seilza_to_line(std::declval<std::string&>()));

	template <typename T>
	using has_seilza_to_line_vt = wws::is_detected<has_seilza_to_line_t, T>;

	template <typename T,typename = std::enable_if_t< has_seilza_form_line_vt<T>::value && has_seilza_to_line_vt<T>::value >>
	class form{	

		std::vector<T> items;
		const char *pform_name;
		std::mutex queue_mut;
		std::queue< std::function<void(std::vector<T>&)> > queue;
		std::thread thread;
		bool th_runing = true;
	public:
		form(const char *pform_name_) : pform_name(pform_name_)
		{
			check_root();
			load();
			start_rw_thread();
		}
		~form()
		{
			wait_rw();
			save();
		}
	protected:
		inline  static const char* root = "forms/";

		void wait_rw()
		{
			queue_mut.lock();

			th_runing = false;

			queue_mut.unlock();
			if (thread.joinable())
				thread.join();
		}

		void check_root()
		{
			namespace fs = std::filesystem;
			std::string sp(root);
			fs::path fp(sp);
			if (!fs::exists(fp))
			{
				fs::create_directory(fp);
			}
		}

		void load()
		{
			namespace fs = std::filesystem;
			std::string sp(root);
			sp += pform_name;
			sp += ".txt";
			fs::path fp(sp);
			if (fs::exists(fp))
			{
				std::ifstream stream(sp,std::ios::in);
				if (stream.good())
				{
					std::string temp_s;
					while (!stream.eof())
					{
						char c = stream.get();
						temp_s += c;
						if (c == '\n')
						{
							T t;
							t.seilza_form_line(temp_s);
							items.push_back(std::move(t));
							temp_s.clear();
						}
					}
				}
				else {
					throw LoadFormExce();
				}
			}
		}

		void save()
		{
			if (items.empty()) return;
			namespace fs = std::filesystem;
			std::string sp(root);
			sp += pform_name;
			sp += ".txt";
			fs::path fp(sp);

			std::ofstream stream(sp, std::ios::out);
			if (stream.good())
			{
				std::string temp_s;
				for (auto& t : items)
				{
					t.seilza_to_line(temp_s);
					stream << temp_s;
				}
			}
			else {
				throw SaveFormExce();
			}
		}
		static void thread_body(form<T>* f)
		{
			bool is_empty = false;
			while (f->th_runing)
			{

				f->queue_mut.lock();
				is_empty = f->queue.empty();
				f->queue_mut.unlock();
					

				if (!is_empty)
				{
					std::lock_guard<std::mutex> lock(f->queue_mut);
					auto func = f->queue.front();
					func(f->items);
					f->queue.pop();
				}
				else {
					std::this_thread::yield();
				}
			}
			std::lock_guard<std::mutex> lock(f->queue_mut);
			while (!f->queue.empty())
			{
				auto func = f->queue.front();
				func(f->items);
				f->queue.pop();
			}
		}
		void start_rw_thread()
		{
			th_runing = true;
			thread = std::thread(form<T>::thread_body,this);
		}
	public:

		void change(std::function<void(std::vector<T>&)> f)
		{
			std::lock_guard<std::mutex> lock(queue_mut);
			queue.push(f);
		}

	};

}