// ChatServer.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
#include <WinSock2.h>
#include <Windows.h>

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min


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
		WSAdata(uint32_t major,uint32_t version) noexcept(false)
		{
			if (instance_count > 0)
			{
				init_success = false;
				throw std::runtime_error("Already has a instance!");
				return;
			}
			++instance_count;
			WORD sockVersion = MAKEWORD(major, version);
			WSADATA wsaData;
			if (WSAStartup(sockVersion, &wsaData) != 0)
			{
				init_success = false;
				throw std::runtime_error("Failed to init WSA!");
				return ;
			}
			else
				init_success = true;
		}
		~WSAdata() {
			if(init_success)
				WSACleanup();
		}

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
		Socket(Socket&& oth)
		{
			fd = oth.fd;
			oth.fd = INVALID_SOCKET;
			ip = std::move(oth.ip);
			port = oth.port;
		}
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&& oth)
		{
			if (fd != INVALID_SOCKET)
			{
				closesocket(fd);
			}
			this->fd = oth.fd;
			ip = std::move(oth.ip);
			port = oth.port;
			oth.fd = INVALID_SOCKET;
			return *this;
		}
		
		Socket accept() noexcept(false)
		{
			sockaddr_in addr;
			int nAddrlen = static_cast<int>(sizeof(addr));
			SOCKET cli_fd = ::accept(fd, (SOCKADDR*)&addr, &nAddrlen);
			if (cli_fd == INVALID_SOCKET)
			{
				throw std::runtime_error("Accept error!");
			}
			auto res = Socket(cli_fd);
			res.set_addr(addr);
			return res;
		}

		bool is_invalid()
		{
			return fd == INVALID_SOCKET;
		}

		int send(const std::string& str) noexcept(false)
		{
			if (!str.empty())
			{
				int ret =::send(fd, str.data(), static_cast<int>(str.size()), 0);
				if (ret == SOCKET_ERROR)
				{
					throw SocketCloseErr();
				}
				return ret;
			}
			return 0;
		}

		int send(char* buf, uint32_t len) noexcept(false)
		{
			if (buf)
			{
				int ret = ::send(fd, buf, len, 0);
				if (ret == SOCKET_ERROR)
				{
					throw SocketCloseErr();
				}
				return ret;
			}
			return 0;
		}

		int send(int v) noexcept(false)
		{
			if (!wws::big_endian())
			{
				v = wws::reverse_byte(v);
			}
			return send(reinterpret_cast<char *>(&v), sizeof(int));
		}

		template<typename T,typename = std::enable_if_t<std::is_integral_v<T>>>
		int recv() noexcept(false)
		{
			int res = 0;
			int ret = recv(reinterpret_cast<char*>(&res), sizeof(T));
			if (ret <= 0)
			{
				throw SocketCloseErr();
			}
			if (ret < sizeof(T))
			{
				throw LessExpectedErr();
			}
			if (!wws::big_endian())
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

		int recv(char *buf,uint32_t len) noexcept(true)
		{
			if (len > 0)
			{
				return ::recv(fd, buf, len, 0);
			}
			return 0;
		}

		void close()
		{
			if (fd != INVALID_SOCKET)
			{
				::closesocket(fd);
				fd = INVALID_SOCKET;
			}
		}

		~Socket()
		{
			if (fd != INVALID_SOCKET)
			{
				::closesocket(fd);
			}
		}
		const std::string& get_ip()
		{
			return ip;
		}
		unsigned short get_port()
		{
			return port;
		}
	protected:
		std::string ip;
		unsigned short port;
		void set_addr(sockaddr_in& addr);

	private:
		Socket(SOCKET fd_) : fd(fd_) {}
		Socket() noexcept(true) {
			fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		
		SOCKET fd;
	};

	
}



// TODO: 在此处引用程序需要的其他标头。
