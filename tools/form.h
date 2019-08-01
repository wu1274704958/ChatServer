#pragma once
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <serialization.hpp>
#include <make_color.hpp>
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
	using has_seilza_form_line_t = decltype(std::declval<T>(std::declval<std::string>()).seilza_form_line);

	template <typename T>										
	using has_seilza_form_line_vt = wws::is_detected<has_seilza_form_line_t, T>;

	template <class T>
	using has_seilza_to_line_t = decltype(std::declval<T>(std::declval<std::string>()).seilza_to_line);

	template <typename T>
	using has_seilza_to_line_vt = wws::is_detected<has_seilza_to_line_t, T>;

	template <typename T,typename = std::enable_if_t< has_seilza_form_line_vt<T>::value && has_seilza_to_line_vt<T>::value >>
	class form{	

		std::vector<T> items;
		const char *pform_name;
	public:
		form(const char *pform_name_) pform_name(pform_name_)
		{
			load();
		}
		~form()
		{
			wait_rw();
			save();
		}
	protected:
		static const char *root = "./form/";

		void wait_rw()
		{

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
	};

}