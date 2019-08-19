#pragma once
#include "ab_client.hpp"
#include <mutex>
#include <vector>
#include <memory>

namespace abc {

	template <typename CLI = ab_client>
	class ab_clients
	{
	public:
		using Client_Ty = std::shared_ptr<CLI>;
		ab_clients()
		{

		}
		ab_clients(const ab_clients<CLI>&) = delete;
		ab_clients(ab_clients<CLI>&&) = delete;
		ab_clients<CLI>& operator=(const ab_clients<CLI>&) = delete;
		ab_clients<CLI>& operator=(ab_clients<CLI>&&) = delete;
		~ab_clients()
		{

		}

		void push_back(std::shared_ptr<CLI>&& p)
		{
			std::lock_guard guard(mux);
			clients.push_back(std::move(p));
		}

		void push_back(std::shared_ptr<CLI>& p)
		{
			std::lock_guard guard(mux);
			clients.push_back(p);
		}

		int size()
		{
			std::lock_guard guard(mux);
			return clients.size();
		}

		std::shared_ptr<CLI> at(int index)
		{
			std::lock_guard guard(mux);
			return clients.at(index);
		}

		auto erase(std::shared_ptr<CLI>& p)
		{
			std::lock_guard guard(mux);
			return clients.erase(std::find(std::begin(clients), std::end(clients), p));
		}

		bool exist_ab_client(int uid, ClientType t = ClientType::Default)
		{
			bool res = false;
			std::lock_guard guard(mux);
			for (auto p : clients)
			{
				if (p->get_uid() == uid && p->get_client_type() == t)
				{
					res = true;
					break;
				}
			}
			return res;
		}

	private:
		std::vector<std::shared_ptr<CLI>> clients;
		std::mutex mux;
	};

	typedef ab_clients<ab_client> def_ab_clients;
}