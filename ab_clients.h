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

	private:
		std::vector<std::shared_ptr<CLI>> clients;
		std::mutex mux;
	};


}