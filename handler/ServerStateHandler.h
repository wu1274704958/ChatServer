#pragma once
#include "Handler.hpp"

namespace handler {
	class ServerStateHandler : public Handler {
	public:
		ServerStateHandler(wws::form<forms::User> &users, abc::def_ab_clients& clients, Client_Ty client) :
			Handler(users, clients, std::move(client))
		{}
		void handle(std::shared_ptr<wws::Json>&&) override;
	};
}