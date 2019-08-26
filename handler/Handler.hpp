#pragma once

#include "../ab_clients.h"
#include <json.hpp>
#include <sqlpp/Connect.h>

namespace handler {
	class Handler {
	protected:
		using Client_Ty = abc::def_ab_clients::Client_Ty;

		sql::Connect &conn;
		abc::def_ab_clients& clients;
		Client_Ty client;
		Handler() = delete;
		Handler(sql::Connect &conn,abc::def_ab_clients& clients, Client_Ty client) :
			conn(conn),
			clients(clients),
			client(std::move(client))
		{}
	public:
		virtual void handle(std::shared_ptr<wws::Json>&&){};
	};
}