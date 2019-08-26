#pragma once
#include "Handler.hpp"

namespace handler {
	class LoginHandler : public Handler {
	public:
		LoginHandler(sql::Connect &conn, abc::def_ab_clients& clients, Client_Ty client) :
			Handler(conn,clients,std::move(client))
		{}
		void handle(std::shared_ptr<wws::Json>&&) override;
	};
}