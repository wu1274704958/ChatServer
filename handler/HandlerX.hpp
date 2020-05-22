#pragma once
#include <tuple>
#include "../constant.h"
#include <memory>
#include <json.hpp>
#include <functional>

namespace handler {

	template <typename T>
	struct is_shared_ptr {
		constexpr static bool val = false;
	};

	template <typename T, template <typename P> class Ptr>
	struct is_shared_ptr<Ptr<T>> {
		constexpr static bool val = std::is_same_v<std::shared_ptr<T>,Ptr<T>>;
	};
	

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
			if constexpr (is_shared_ptr<T>::val)
			{
				t-> template send_error<EC, C>();
			}
			else
			{
				t. template send_error<EC, C>();
			}
		}

		template<abc::ErrorCode EC, typename T>
		void error(T& t,wws::Json&& data)
		{
			if constexpr (is_shared_ptr<T>::val)
			{
				t-> template send_error<EC, C>(std::move(data));
			}
			else
			{
				t. template send_error<EC, C>(std::move(data));
			}
		}
		
	private:
		Args_Ty args;
	};

}