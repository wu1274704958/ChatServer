#pragma once

#include "HandlerX.hpp"
#include "../ab_clients.h"
#include <sqlpp/Connect.h>

namespace handler {
	class ModifyInfo : public HandlerX<abc::HandlerCode::ModifyInfo,
		std::reference_wrapper<abc::def_ab_clients>,
		abc::def_ab_clients::Client_Ty,
		std::reference_wrapper<sql::Connect>>
	{
	public:
		ModifyInfo(Args_Ty args) : HandlerX(std::move(args))
		{

		}

		void handle(std::shared_ptr<wws::Json>&&) override;

	private:

	};
}