#pragma once
#include "socket.h"
#include <mutex>
#include <json.hpp>
#include <tuple>
#include <memory>
#include <atomic>
#include <dbg.hpp>

namespace abc
{

	enum class ErrorCode : int{
		Success				= 0,
		ArgsError			= -1,
		BadRequest			= -2,
		CanNotHandle		= -3,
		IncorrectPassword	= -4,
		IncorrectAccount	= -5,
		PermissionDenied	= -6,
		AlreadyLogged		= -7,
		Failed				= -8,
		AlreadyRegister		= -9,
		NotLogin			= -10

	};

	enum class HandlerCode : unsigned int
	{
		NoHandler = 0x80000000,
		Invaild = 0x7fffffff,
		Register = 0,
		Test = 1,
		Login = 2,
		Logout,
	};

	enum class ClientType : int
	{
		NotKnow = -1,
		Default = 0,
		Upload = 1,
		Download = 2,
		P2P = 3,
		Admin = 4
	};

	extern std::vector<std::pair<const char *, HandlerCode>> HandlerMap;
	constexpr int MAX_BYTE_SIZE = 1024 * 1024 * 2;
	constexpr int INVALID_UID = -1;

	struct TooBigPkg : public std::exception{
		TooBigPkg() : std::exception("Too big info package!") {}
	};

	class ab_client
	{
	public:
		ab_client(sock::Socket& s) 
			: socket(std::move(s))
		{

		}
		~ab_client()
		{

		}

		std::tuple<HandlerCode,std::shared_ptr<wws::Json>> wait_request()
		{
			std::string req;
			try {
				std::lock_guard guard(r_mutex);
				int len = socket.recv<int>();
				if (len > MAX_BYTE_SIZE || len <= 0)
					throw TooBigPkg();
				req = socket.recv(len);

			}
			catch (sock::LessExpectedErr e)
			{
				dbg(e.what());
				send_error<ErrorCode::BadRequest>();
				return std::make_tuple(HandlerCode::Invaild,std::shared_ptr<wws::Json>(nullptr));
			}
			try {
				wws::Json reqj(req);
				std::string name = reqj.get<std::string>("reqn");
				if (reqj.has_key("data"))
				{
					return std::make_tuple( get_handler_code(name), std::shared_ptr<wws::Json>( new wws::Json(reqj.detach_obj("data"))));
				}
				return std::make_tuple(get_handler_code(name), std::shared_ptr<wws::Json>(nullptr));
			}
			catch (wws::BadKeyErr e)
			{
				dbg(e.what());
				send_error<ErrorCode::ArgsError>();
				return std::make_tuple(HandlerCode::Invaild, std::shared_ptr<wws::Json>(nullptr));
			}
			catch (wws::BadJsonErr e)
			{
				dbg(e.what());
				send_error<ErrorCode::BadRequest>();
				return std::make_tuple(HandlerCode::Invaild, std::shared_ptr<wws::Json>(nullptr));
			}
		}
		
		HandlerCode get_handler_code(std::string& name)
		{
			for (auto& pa : HandlerMap)
			{
				if (name == pa.first)
				{
					return pa.second;
				}
			}
			return HandlerCode::NoHandler;
		}

		template <ErrorCode code>
		void send_error(wws::Json data)
		{
			wws::Json ret;
			ret.put("ret",static_cast<int>(code));
			ret.put("data", std::move(data));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			socket.send(static_cast<int>(str.size()) );
			socket.send(str);
		}

		template <ErrorCode code>
		void send_error()
		{
			wws::Json ret;
			ret.put("ret", static_cast<int>(code));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			socket.send(static_cast<int>(str.size()));
			socket.send(str);
		}

		ClientType get_client_type()
		{
			return client_type;
		}

		void set_client_type(ClientType t)
		{
			client_type = t;
		}

		int get_uid()
		{
			return uid;
		}

		void set_uid(int v)
		{
			uid = v;
		}

		std::lock_guard<std::mutex> get_w_guard()
		{
			return std::lock_guard(w_mutex);
		}

		std::lock_guard<std::mutex> get_r_guard()
		{
			return std::lock_guard(r_mutex);
		}


	private:
		sock::Socket socket;
		std::mutex r_mutex;
		std::mutex w_mutex;
		std::atomic<ClientType> client_type;
		std::atomic<int> uid = INVALID_UID;
	};

}