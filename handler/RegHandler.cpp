#include "RegHandler.h"

using namespace abc;
using namespace wws;
using namespace forms;

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
	if (user.good_acc() && user.good_psd() && user.good_sex())
	{
		if (users.has_by_fields(std::make_tuple(user.acc), &User::acc) > -1)
		{
			client->send_error<ErrorCode::AlreadyRegister, HandlerCode::Register>();
			return;
		}

		int uid = 0;
		users.change([&uid,&user](std::vector<User> us) {
			if (us.empty())
			{
				uid = User::MIN_UID;
			}
			else {
				uid = us.back().id + 1;
			}
		});
		user.id = uid;
		users.push_back(std::move(user));
		client->send_error<ErrorCode::Success, HandlerCode::Register>();
	}
	else {
		client->send_error<ErrorCode::Failed, HandlerCode::Register>();
	}

}
