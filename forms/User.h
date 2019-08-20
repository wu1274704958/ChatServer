#pragma once
#include <string>
#include <vector>
#include <serialization.hpp>


namespace forms {

	enum class SexType : unsigned char {
		Boy = 65,
		Girl,
		Both
	};

	class User {
	public:
		int			id;
		bool		is_admin;
		char		sex;
		uint32_t	age;
		std::string name;
		std::string acc;
		std::string psd;
		std::string	head;
		std::vector<uint32_t> friends;
		constexpr static int MIN_UID = 10000;
	
		User() = default;

		User(int id_,
			bool is_admin_,
			uint32_t age_,
			SexType sex,
			std::string& name_,
			std::string& acc_,
			std::string& psd_,
			std::string& head,
			std::vector<uint32_t>& friends_) :
			id(id_),
			is_admin(is_admin_),
			age(age_),
			sex(static_cast<char>(sex)),
			name(name_),
			acc(acc_),
			psd(psd_),
			head(head),
			friends(friends_){}

		User(int id_,
			bool is_admin,
			uint32_t age_,
			SexType sex,
			const char* name_,
			const char* acc_,
			const char* psd_,
			const char* head,
			std::vector<uint32_t> friends_) :
			id(id_),
			is_admin(is_admin),
			age(age_),
			sex(static_cast<char>(sex)),
			name(name_),
			acc(acc_),
			psd(psd_),
			head(head),
			friends(friends_) {}

		User(const User&) = default;
		User(User&&) = default;
		User& operator=(const User&) = default;
		User& operator=(User&&) = default;

		void seilza_form_line(std::string& str)
		{
			wws::seilza_form_line(str, *this, &User::id, &User::age, &User::sex, &User::name, &User::acc, 
				&User::psd, &User::friends, &User::is_admin,&User::head);
		}

		void seilza_to_line(std::string& str)
		{
			str = wws::seilza_to_line(*this, &User::id, &User::age, &User::sex, &User::name, &User::acc,
				&User::psd, &User::friends, &User::is_admin, &User::head);
		}

		SexType get_sex()
		{
			return static_cast<SexType>(sex);
		}

		bool good_sex()
		{
			return	
				sex == static_cast<char>(SexType::Boy) ||
				sex == static_cast<char>(SexType::Girl) ||
				sex == static_cast<char>(SexType::Both);
		}

		bool good_acc()
		{
			if (acc.size() < 9)
				return false;
			for (auto c : acc)
			{
				if (!((c >= 48 && c <= 57) || (c >= 65 && c <= 106) || (c >= 97 && c <= 122)))
				{
					return false;
				}
			}
			return true;
		}

		bool good_psd()
		{
			if (psd.size() < 9)
				return false;
			return true;
		}

		friend inline std::ostream& operator<<(std::ostream& out, User& u);
	};

	std::ostream& operator<<(std::ostream& out, User& u)
	{
		out << wws::seilza_to_line(u, &User::id, &User::age, &User::sex, &User::name, &User::acc,
			&User::psd, &User::friends, &User::is_admin, &User::head);
		return out;
	}

}