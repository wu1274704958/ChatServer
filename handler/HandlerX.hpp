#pragma once
#include <tuple>
#include "../constant.h"
#include <memory>
#include <json.hpp>

namespace handler {

	template<abc::HandlerCode C,typename ...T>
	class HandlerX {
	public:
		using Args_Ty = std::tuple<T...>;
		HandlerX(Args_Ty args) : args(std::move(args)) {}
		virtual void handle(std::shared_ptr<wws::Json>&&) = 0;
		Args_Ty& get_args()
		{
			return args;
		}
		template<abc::ErrorCode EC,typename T>
		void error(T& t)
		{
			t.template send_error<EC,C>();
		}
		
	private:
		Args_Ty args;
	};

}