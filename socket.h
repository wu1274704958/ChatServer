#pragma once
// ChatServer.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。




#include <iostream>
#include <comm.hpp>

namespace sock{

	class SocketCloseErr : public std::runtime_error 
	{
	public:
		SocketCloseErr(): std::runtime_error("Socket closed!")  {
			
		}
	};
	class LessExpectedErr : public std::runtime_error
	{
	public:
		LessExpectedErr() : std::runtime_error("Bytes less than expected!") {

		}
	};

	

	class WSAdata
	{
	public:
		WSAdata(uint32_t major, uint32_t version) noexcept(false);
		~WSAdata();

	private:
		static uint32_t instance_count;
		bool init_success = false;
	};


	class Socket {
		
	public:
		static Socket server(uint16_t port, uint32_t backlog);
		static Socket client(const char *ip, uint16_t port);
		static Socket invalid() noexcept(true);

		Socket(const Socket&) = delete;
		Socket(Socket&& oth);
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&& oth);
		
		Socket accept();

		bool is_invalid();

		int send(const std::string& str) noexcept(false);

		int send(char* buf, uint32_t len) noexcept(false);

		template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
		int send(T v) noexcept(false)
		{
			if (sizeof(T) > 1 && !wws::big_endian())
			{
				v = wws::reverse_byte(v);
			}
			return send(reinterpret_cast<char *>(&v), sizeof(T));
		}

		template<typename T,typename = std::enable_if_t<std::is_integral_v<T>>>
		T recv() noexcept(false)
		{
			T res = static_cast<T>(0);
			int ret = recv(reinterpret_cast<char*>(&res), sizeof(T));
			if (ret <= 0)
			{
				throw SocketCloseErr();
			}
			if (ret < sizeof(T))
			{
				throw LessExpectedErr();
			}
			if (sizeof(T) > 1 && !wws::big_endian())
			{
				res = wws::reverse_byte(res);
			}
			return res;
		}

		std::string recv(uint32_t len)
		{
			std::string res;
			if (len > 0)
			{
				res.resize(len);
				int ret = recv(const_cast<char *>(res.data()),len);
				if (ret < 0)
				{
					throw SocketCloseErr();
				}
				if (ret < len)
				{
					throw LessExpectedErr();
				}
			}
			return res;
		}

		int recv(char* buf, uint32_t len) noexcept(true);

		void close();

		~Socket();

		const std::string& get_ip()
		{
			return ip;
		}
		unsigned short get_port()
		{
			return port;
		}
	protected:
		template<typename S>
		void set_native_socket(S t);
		template<typename S>
		S get_native_socket();
	
		std::string ip;
		unsigned short port;
		template<typename Addr>
		void set_addr(Addr& addr);

	private:
		Socket(void* fd_) : fd(fd_) {}
		Socket() noexcept(true);
		
		void* fd;
	};

	
}



// TODO: 在此处引用程序需要的其他标头。
