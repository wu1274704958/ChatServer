#pragma once
#include "Handler.hpp"

namespace handler {
	class ServerStateHandler : public Handler {
	public:
		ServerStateHandler(sql::Connect &conn, abc::def_ab_clients& clients, Client_Ty client) :
			Handler(conn, clients, std::move(client))
		{}
		void handle(std::shared_ptr<wws::Json>&&) override;
	};
}