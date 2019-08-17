#pragma once
#include "ab_client.hpp"
#include <mutex>
#include <vector>
#include <memory>

namespace abc {

	template <typename CLI = ab_client>
	class clients
	{
	public:
		clients()
		{

		}
		clients(const clients<CLI>&) = delete;
		clients( clients<CLI>&&) = delete;
		clients<CLI>& operator=(const clients<CLI>&) = delete;
		clients<CLI>& operator=(clients<CLI>&&) = delete;
		~clients()
		{

		}

		void push_back(std::shared_ptr<CLI>&& p)
		{
			std::lock_guard guard(mux);
			cls.push_back(std::move(p));
		}

		int size()
		{
			std::lock_guard guard(mux);
			return cls.size();
		}

		std::shared_ptr<CLI> at(int index)
		{
			std::lock_guard guard(mux);
			return cls.at(index);
		}

	private:
		std::vector<std::shared_ptr<CLI>> cls;
		std::mutex mux;
	};


}