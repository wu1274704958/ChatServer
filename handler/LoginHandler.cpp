#include "LoginHandler.h"

using namespace abc;
using namespace wws;
using namespace forms;

void handler::LoginHandler::handle(std::shared_ptr<Json>&& data_ptr)
{
	if (client->get_client_type() != ClientType::NotKnow)
	{
		client->send_error<ErrorCode::AlreadyLogged>();
		return;
	}
	if (!data_ptr)
	{
		client->send_error<ErrorCode::ArgsError>();
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
			client->send_error<ErrorCode::ArgsError>();
			return;
		}

		users.change([this, acc = std::move(acc), psd = std::move(psd)](std::vector<forms::User>& us) mutable {
			bool has = false;
			bool right_psd = false;

			for (auto& u : us)
			{
				if (u.acc == acc)
				{
					right_psd = u.psd == psd;
					if (right_psd)
					{
						if (clients.exist_ab_client(u.id, u.is_admin ? ClientType::Admin : ClientType::Default))
						{
							client->send_error<ErrorCode::AlreadyLogged>();
							return;
						}

						this->client->set_uid(u.id);
						if (u.is_admin)
							client->set_client_type(ClientType::Admin);
						else
							client->set_client_type(ClientType::Default);
						wws::Json dat = toJson(u,
							{ "id","acc","psd","age","is_admin","name","friends" },
							&User::id, &User::acc, &User::psd, &User::age, &User::is_admin, &User::name, &User::friends);
						client->send_error<ErrorCode::Success>(std::move(dat));
						return;
					}
					else {
						client->send_error<ErrorCode::IncorrectPassword>();
						return;
					}
				}
			}
			client->send_error<ErrorCode::IncorrectAccount>();
		});
	}
}
