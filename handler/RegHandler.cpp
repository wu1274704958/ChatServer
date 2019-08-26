#include "RegHandler.h"
#include "../forms/User.h"
#include <sqlpp/mysql.hpp>
using namespace abc;
using namespace wws;
using namespace forms;
using namespace sql;

void handler::RegHandler::handle(std::shared_ptr<wws::Json>&& data_ptr)
{
	if (!data_ptr)
	{
		client->send_error<ErrorCode::ArgsError,HandlerCode::Register>();
		return;
	}
	if (client->get_client_type() != ClientType::NotKnow && client->get_client_type() != ClientType::Admin)
	{
		client->send_error<ErrorCode::Failed, HandlerCode::Register>();
		return;
	}

	User user;

	try {
		formJson(user, *data_ptr, { "acc","psd","sex","age","name" }, &User::acc, &User::psd, &User::sex, &User::age, &User::name);
	}
	catch (BadKeyErr e)
	{
		dbg(e.what());
		client->send_error<ErrorCode::ArgsError,HandlerCode::Register>();
		return;
	}

	user.is_admin = false;
	user.uid = 0;
	if (user.good_acc() && user.good_psd() && user.good_sex())
	{
		std::string acc_ = user.acc;
		Query q;
		Result res = q.asc<User, K::select, K::star, K::from>()
			.where<K::eq, true>(&User::acc, std::move(acc_))
			.exec(conn);
		q.clear();
		if (res.rows() > 0)
		{
			client->send_error<ErrorCode::AlreadyRegister, HandlerCode::Register>();
			return;
		}
		else {
			q.insert(user, &User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends).exec(conn);
			
			if (conn.affected_rows() == 1)
			{
				q.clear();
				res = q.select<User>(&User::uid)
					.where<K::eq, true>(&User::acc, std::move(user.acc))
					.exec(conn);
				if (res.rows() == 1)
				{
					Row r = res.next();
					auto[id] = r.get_tup<int>();
					std::cout << "New register id = " << id << std::endl;
					//Json data;
					//data.put("uid", id);
					client->send_error<ErrorCode::Success, HandlerCode::Register>();
				}
				else {
					client->send_error<ErrorCode::UnKnowErr, HandlerCode::Register>();
					return;
				}
			}
			else {
				client->send_error<ErrorCode::UnKnowErr, HandlerCode::Register>();
				return;
			}
		}
	}
	else {
		client->send_error<ErrorCode::Failed, HandlerCode::Register>();
	}

}
