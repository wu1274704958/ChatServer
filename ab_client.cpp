#include "ab_client.hpp"

#include <vector>

namespace abc {
	std::vector<std::pair<const char *, HandlerCode>> HandlerMap = {
		{"Register",HandlerCode::Register},
		{"Test",HandlerCode::Test},
		{"Login",HandlerCode::Login},
		{"Logout",HandlerCode::Logout},
		{"ServerState",HandlerCode::ServerState},
		{"Heart",HandlerCode::Heart}
	};
}