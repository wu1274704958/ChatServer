#pragma once

#include "../ab_clients.h"
#include "../forms/User.h"
#include "../tools/form.h"
#include <json.hpp>

namespace handler {
	class Handler {
	protected:
		using Client_Ty = abc::def_ab_clients::Client_Ty;

		wws::form<forms::User> &users;
		abc::def_ab_clients& clients;
		Client_Ty client;
		Handler() = delete;
		Handler(wws::form<forms::User> &users,abc::def_ab_clients& clients, Client_Ty client) :
			users(users),
			clients(clients),
			client(std::move(client))
		{}
	public:
		virtual void handle(std::shared_ptr<wws::Json>&&){};
	};
}