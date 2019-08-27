#include "ab_client.hpp"

#include <vector>

#define Def_Handler_(s) static_cast<unsigned int>(HandlerCode::##s)	,HandlerCode::##s

namespace abc {
	std::vector<std::pair<unsigned int, HandlerCode>> HandlerMap = {
		{Def_Handler_(Register)		},
		{Def_Handler_(Test)			},
		{Def_Handler_(Login)		},
		{Def_Handler_(Logout)		},
		{Def_Handler_(ServerState)	},
		{Def_Handler_(Heart)		},
		{Def_Handler_(ModifyInfo)   }
	};
}