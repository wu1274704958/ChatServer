#pragma once
#include <string>
#include <vector>
#include <serialization.hpp>


namespace forms {
	class User {
	public:
		int			id;
		bool		is_admin;
		uint32_t	age;
		std::string name;
		std::string acc;
		std::string psd;
		std::vector<uint32_t> friends;

	
		User() = default;

		User(int id_,
			bool is_admin_,
			uint32_t age_,
			std::string& name_,
			std::string& acc_,
			std::string& psd_,
			std::vector<uint32_t>& friends_) :
			id(id_),
			is_admin(is_admin_),
			age(age_),
			name(name_),
			acc(acc_),
			psd(psd_),
			friends(friends_){}

		User(int id_,
			bool is_admin,
			uint32_t age_,
			const char* name_,
			const char* acc_,
			const char* psd_,
			std::vector<uint32_t> friends_) :
			id(id_),
			is_admin(is_admin),
			age(age_),
			name(name_),
			acc(acc_),
			psd(psd_),
			friends(friends_) {}

		User(const User&) = default;
		User(User&&) = default;
		User& operator=(const User&) = default;
		User& operator=(User&&) = default;

		void seilza_form_line(std::string& str)
		{
			wws::seilza_form_line(str, *this, &User::id, &User::age, &User::name, &User::acc, &User::psd, &User::friends, &User::is_admin);
		}

		void seilza_to_line(std::string& str)
		{
			str = wws::seilza_to_line(*this, &User::id, &User::age, &User::name, &User::acc, &User::psd, &User::friends, &User::is_admin);
		}

		friend inline std::ostream& operator<<(std::ostream& out, User& u);
	};

	std::ostream& operator<<(std::ostream& out, User& u)
	{
		out << wws::seilza_to_line(u, &User::id, &User::age, &User::name, &User::acc, &User::psd, &User::friends, &User::is_admin);
		return out;
	}

}