#pragma once
#include "socket.h"
#include <mutex>
#include <json.hpp>
#include <tuple>
#include <memory>
#include <atomic>
#include <dbg.hpp>
#include <chrono>
#include "constant.h"

namespace abc
{
	extern std::vector<std::pair<unsigned int, HandlerCode>> HandlerMap;

	struct TooBigPkg : public std::exception{
		TooBigPkg() : std::exception("Too big info package!") {}
	};
	struct BadDataPkg : public std::exception {
		BadDataPkg() : std::exception("Bad Data package!") {}
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
			int CRC = 0;
			try {
				std::lock_guard guard(r_mutex);
				
				while (socket.recv<unsigned char>() != BeginBit){}
				
				int len = socket.recv<int>();
				if (len > MAX_BYTE_SIZE || len <= 0)
				{
					send_error<ErrorCode::TooBigDataPkg>();
					throw TooBigPkg();
				}
				req = socket.recv(len);
				CRC = socket.recv<int>();
				if (socket.recv<unsigned char>() != EndBit)
				{
					send_error<ErrorCode::BadDataPkg>();
					throw BadDataPkg();
				}
			}
			catch (sock::LessExpectedErr e)
			{
				dbg(e.what());
				send_error<ErrorCode::BadRequest>();
				return std::make_tuple(HandlerCode::Invaild,std::shared_ptr<wws::Json>(nullptr));
			}
			try {
				wws::Json reqj(req);
				unsigned int name = reqj.get<unsigned int>("reqn");
				if (reqj.has_key("data"))
				{
					return std::make_tuple(get_handler_code( name ), std::shared_ptr<wws::Json>( new wws::Json(reqj.detach_obj("data"))));
				}
				return std::make_tuple(get_handler_code( name ), std::shared_ptr<wws::Json>(nullptr));
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
		
		HandlerCode get_handler_code(unsigned int name)
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
			ret.put("repo", static_cast<unsigned int>(HandlerCode::Invaild));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			send(str);
		}

		template <ErrorCode code>
		void send_error()
		{
			wws::Json ret;
			ret.put("ret", static_cast<int>(code));
			ret.put("repo", static_cast<unsigned int>(HandlerCode::Invaild));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			send(str);
		}

		template <ErrorCode code,HandlerCode Repo>
		void send_error()
		{
			wws::Json ret;
			ret.put("ret", static_cast<int>(code));
			ret.put("repo", static_cast<unsigned int>(Repo));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			send(str);
		}

		template <ErrorCode code, HandlerCode Repo>
		void send_error(wws::Json data)
		{
			wws::Json ret;
			ret.put("ret", static_cast<int>(code));
			ret.put("data", std::move(data));
			ret.put("repo", static_cast<unsigned int>(Repo));
			std::string str = ret.to_string();

			std::lock_guard guard(w_mutex);
			send(str);
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

		std::string get_ip()
		{
			std::lock_guard guard(socket_mux);
			return socket.get_ip();
		}

		unsigned short get_port()
		{
			std::lock_guard guard(socket_mux);
			return socket.get_port();
		}

		void set_heart()
		{
			namespace sc = std::chrono;
			heart_time = sc::system_clock::now();
		}

		long long last_heart_duration()
		{
			namespace sc = std::chrono;
			auto dur = sc::system_clock::now() - heart_time.load();
			return sc::duration_cast<sc::milliseconds>(dur).count();
		}

		void close()
		{
			std::lock_guard guard(socket_mux);
			socket.close();
		}

		bool is_invalid()
		{
			std::lock_guard guard(socket_mux);
			return socket.is_invalid();
		}

		bool is_busy()
		{
			return busy;
		}

		class busy_guard {
		public:
			busy_guard(ab_client& cli) : cli(cli) { cli.busy = true; }
			~busy_guard() { cli.busy = false; }
			busy_guard(const busy_guard&) = delete;
			busy_guard& operator=(const busy_guard&) = delete;
		private:
			ab_client& cli;
		};

		busy_guard get_busy_guard()
		{
			return busy_guard(*this);
		}

	private:
		void send(std::string& data)
		{
			socket.send<unsigned char>(BeginBit);
			socket.send<int>(data.size());
			socket.send(data);
			socket.send<int>(0x0);
			socket.send<unsigned char>(EndBit);
		}

		inline void send(std::string&& data)
		{
			send(data);
		}

	private:
		sock::Socket socket;
		std::mutex socket_mux;//except send and recv 
		std::mutex r_mutex;
		std::mutex w_mutex;
		std::atomic<ClientType> client_type;
		std::atomic<int> uid = INVALID_UID;
		std::atomic<std::chrono::time_point<std::chrono::system_clock>> heart_time;
		std::atomic<bool> busy = false;
	};

}