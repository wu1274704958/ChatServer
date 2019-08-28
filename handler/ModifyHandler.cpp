#include "ModifyHandler.h"
#include <sqlpp/mysql.hpp>
#include "../forms/User.h"

using namespace abc;
using namespace sql;
using namespace wws;
using namespace forms;

namespace handler {

	void ModifyInfo::handle(std::shared_ptr<wws::Json>&& data)
	{
		char type = -1;
		auto[clients,client,conn] = get_args();
		switch (client->get_client_type())
		{
			case ClientType::Admin:
				type = 1;
				break;
			case ClientType::Default:
				type = 0;
				break;
			default:
				break;
		}
		if (type < 0)
		{
			error<ErrorCode::NotLogin>(*client);
			return;
		}
		if(!data)
		{
			error<ErrorCode::ArgsError>(*client);
			return;
		}
		int id = INVALID_UID;

		try {
			if (type == 1)
			{
				if (data->has_key("uid"))
				{
					id = data->get<int>("uid");
				}
				else
					id = client->get_uid();
			}
			else {
				id = client->get_uid();
			}

			Query q;
			Result res = q.select(&User::psd)
				.where<K::eq, false>(&User::uid, to_string(id))
				.exec(conn);

			if (res.rows() == 1)
			{
				Row r = res.next();
				auto [r_psd] = r.get_tup<std::string>();
				std::string psd = data->get<std::string>("psd");
				if (r_psd != psd)
				{
					error<ErrorCode::IncorrectPassword>(*client);
					return;
				}

				q.clear();
				q.asc<User, K::update>()
					.a<K::set>();

				User temp;

				if (data->has_key(User::get_field_name(&User::acc)))
				{
					temp.acc = data->get<std::string>(User::get_field_name(&User::acc));
					if (!temp.good_acc())
					{
						error<ErrorCode::Failed>(*client);
						return;
					}
					q.a(&User::acc)
						.a<K::eq, true, true, ' ', ','>(std::move(temp.acc));
				}
				if (data->has_key(User::get_field_name(&User::psd)))
				{
					temp.psd = data->get<std::string>(User::get_field_name(&User::psd));
					if (!temp.good_psd())
					{
						error<ErrorCode::Failed>(*client);
						return;
					}
					q.a(&User::psd)
						.a<K::eq, true, true, ' ', ','>(std::move(temp.psd));
				}
				if (data->has_key(User::get_field_name(&User::name)))
				{
					q.a(&User::name)
						.a<K::eq, true, true, ' ', ','>(data->get<std::string>(User::get_field_name(&User::name)));
				}
				if (data->has_key(User::get_field_name(&User::age)))
				{
					std::string age = data->get_str(User::get_field_name(&User::age));
					q.a(&User::age)
						.a<K::eq, true,false, ' ', ','>(std::move(age));
				}
				if (data->has_key(User::get_field_name(&User::sex)))
				{
					unsigned char sex = data->get<unsigned char>(User::get_field_name(&User::sex));
					temp.sex = sex;
					if (!temp.good_sex())
					{
						error<ErrorCode::Failed>(*client);
						return;
					}
					q.a(&User::age)
						.a<K::eq, true, false, ' ', ','>(to_string(temp.sex));
				}

				q.str().pop_back();
				q.str().push_back(' ');

				q.where<K::eq, false>(&User::uid, to_string(id))
					.exec(conn);
				if (conn.get().affected_rows() == 1)
					error<ErrorCode::Success>(*client);
				else
					error<ErrorCode::Failed>(*client);
			}
			else {
				error<ErrorCode::Failed>(*client);
				return;
			}
		}
		catch (std::exception e)
		{
			dbg(e.what());
			error<ErrorCode::ArgsError>(*client);
			return;
		}
	}

}
