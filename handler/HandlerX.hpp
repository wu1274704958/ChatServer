#pragma once
#include <tuple>

namespace handler {

	template<typename ...T>
	class HandlerX {
	public:
		using Args_Ty = std::tuple<T&...>;
		HandlerX(Args_Ty args) : args(std::move(args)) {}
		virtual void handle() = 0;
		Args_Ty& get_args()
		{
			return args;
		}
	private:
		Args_Ty args;
	};

}