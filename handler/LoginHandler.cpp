#include "LoginHandler.h"
#include <sqlpp/mysql.hpp>
#include "../forms/User.h"

using namespace abc;
using namespace wws;
using namespace sql;
using namespace forms;

void handler::LoginHandler::handle(std::shared_ptr<Json>&& data_ptr)
{
	if (client->get_client_type() != ClientType::NotKnow)
	{
		client->send_error<ErrorCode::AlreadyLogged, HandlerCode::Login>();
		return;
	}
	if (!data_ptr)
	{
		client->send_error<ErrorCode::ArgsError, HandlerCode::Login>();
		return;
	}
	else {
		std::string acc;
		std::string psd;
		try {
			acc = data_ptr->get<std::string>("acc");
			psd = data_ptr->get<std::string>("psd");
		}
		catch (wws::BadKeyErr e)
		{
			client->send_error<ErrorCode::ArgsError, HandlerCode::Login>();
			return;
		}

		Query q;
		Result res = q.asc<User, K::select, K::star, K::from>()
			.where<K::eq, true>(&User::acc, std::move(acc))
			.exec(conn);
		
		if (res.rows() == 1)
		{
			 Row r = res.next();
			 User u = r.get<User>(&User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends);
			 if (u.psd == psd)
			 {
				 if (clients.exist_ab_client(u.uid, u.is_admin ? ClientType::Admin : ClientType::Default))
				 {
					 client->send_error<ErrorCode::AlreadyLogged, HandlerCode::Login>();
				 }
				 else
				 {
					 Json data = wws::toJson(u, { "uid","is_admin","sex","age","name","acc","psd","head","friends" },
						 &User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends);
					 client->set_uid(u.uid);
					 client->set_client_type(u.is_admin ? ClientType::Admin : ClientType::Default);
					 client->send_error<ErrorCode::Success, HandlerCode::Login>(std::move(data));
				 }
			 }
			 else {
				 client->send_error< ErrorCode::IncorrectPassword, HandlerCode::Login>();
			 }
		}
		else {
			client->send_error< ErrorCode::IncorrectAccount, HandlerCode::Login>();
			return;
		}
	}
}
