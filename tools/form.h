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
#include <chrono>
#include <tuple>

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
		std::mutex mux;
	public:
		form(const char *pform_name_) : pform_name(pform_name_)
		{
			check_root();
			load();
		}
		~form()
		{
			std::lock_guard<std::mutex> lock(mux);
			save();
		}
	protected:
		inline  static const char* root = "forms/";

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
		
	public:

		void change(std::function<void(std::vector<T>&)> f)
		{
			std::lock_guard<std::mutex> lock(mux);
			f(items);
		}

		auto size() -> decltype(std::declval<std::vector<T>>().size())
		{
			std::lock_guard<std::mutex> lock(mux);
			return items.size();
		}

		void push_back(T&& t)
		{
			std::lock_guard<std::mutex> lock(mux);
			items.push_back(std::move(t));
		}

		void push_back(T& t)
		{
			std::lock_guard<std::mutex> lock(mux);
			items.push_back(t);
		}

		bool empty()
		{
			std::lock_guard<std::mutex> lock(mux);
			return items.empty();
		}

		template<typename ...Fs>
		int has_by_fields(std::tuple<Fs...>&& tup,Fs T::* ...fs)
		{
			static_assert(sizeof...(Fs) > 0);
			int res = -1;
			std::lock_guard<std::mutex> lock(mux);
			for (int i = 0; i < items.size();++i)
			{
				if (has_by_fields_sub<0, std::tuple<Fs...> ,Fs...>(items[i], tup, fs ...))
				{
					res = i;
				}
			}

			return res;
		}

	private:
		template<int I,typename TUP,typename Fir,typename ...Fs>
		bool has_by_fields_sub(T& t,TUP& tup, Fir T::*fir, Fs T::* ...fs)
		{
			bool res = (t.*fir == std::get<I>(tup));
			if constexpr(sizeof...(Fs) > 0)
			{
				return res && has_by_fields_sub<I + 1, Fs...>(t, tup, fs...);
			}
			return res;
		}
	};

}