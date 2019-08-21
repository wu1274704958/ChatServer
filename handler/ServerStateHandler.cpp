#include "ServerStateHandler.h"
#include <array>

using namespace abc;
using namespace wws;
using namespace forms;

void handler::ServerStateHandler::handle(std::shared_ptr<wws::Json>&&)
{
	if (client->get_client_type() != ClientType::Admin)
	{
		client->send_error<ErrorCode::PermissionDenied>();
		return;
	}
	std::vector<int> uids;
	std::vector<std::string> addrs;

	clients.change([&uids, &addrs](std::vector<std::shared_ptr<ab_client>> cs)
	{
		for (auto p : cs)
		{
			uids.push_back(p->get_uid());
			std::string ip = p->get_ip();
			ip += ':';
			ip += std::to_string((int)p->get_port());
			addrs.push_back(std::move(ip));
		}
	});

	std::array<int, 3> ts = clients.info<ClientType::Admin, ClientType::Default, ClientType::NotKnow>();
	wws::Json data;

	data.put("olusers", uids);
	data.put("addrs", addrs);
	data.put("admin", std::get<0>(ts));
	data.put("ordinary", std::get<1>(ts));
	data.put("Idler", std::get<2>(ts));

	client->send_error<ErrorCode::Success>(std::move(data));
}
