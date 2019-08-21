#pragma once
#include "ab_client.hpp"
#include <mutex>
#include <vector>
#include <memory>
#include <functional>

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
		template<ClientType ...Ct>
		auto info()->std::array<int, sizeof...(Ct)>
		{
			std::array<int, sizeof...(Ct)> res;
			res.fill(0);
			
			std::lock_guard guard(mux);
			for (auto p : clients)
			{
				client_type_num<0, std::array<int, sizeof...(Ct)>, Ct...>(res, *p);
			}
			return res;
		}

		std::vector<int> all_login_uid()
		{
			std::vector<int> res;

			std::lock_guard guard(mux);
			for (auto p : clients)
			{
				if (p->get_uid() != INVALID_UID)
				{
					res.push_back(p->get_uid());
				}
			}
			return res;
		}

		void change(std::function<void(std::vector<std::shared_ptr<CLI>>&)> f)
		{
			std::lock_guard guard(mux);
			f(clients);
		}

	private:
		template<size_t I,typename Arr,ClientType F, ClientType ...Ct>
		void client_type_num(Arr& arr,CLI& cli)
		{
			if (cli.get_client_type() == F)
				++std::get<I>(arr);
			if constexpr (sizeof...(Ct) > 0)
			{
				client_type_num<I + 1, Arr, Ct...>(arr, cli);
			}
		}

		std::vector<std::shared_ptr<CLI>> clients;
		std::mutex mux;
	};

	typedef ab_clients<ab_client> def_ab_clients;
}