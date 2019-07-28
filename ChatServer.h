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


namespace sock{
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

		Socket(const Socket&) = delete;
		Socket(Socket&& oth)
		{
			fd = oth.fd;
			oth.fd = INVALID_SOCKET;
		}
		Socket& operator=(const Socket&) = delete;
		Socket& operator=(Socket&& oth)
		{
			if (fd != INVALID_SOCKET)
			{
				closesocket(fd);
			}
			this->fd = oth.fd;
			oth.fd = INVALID_SOCKET;
			return *this;
		}
		
		Socket accept(sockaddr_in& addr) noexcept(false)
		{
			int nAddrlen = static_cast<int>(sizeof(addr));
			SOCKET cli_fd = ::accept(fd, (SOCKADDR*)&addr, &nAddrlen);
			if (cli_fd == INVALID_SOCKET)
			{
				throw std::runtime_error("Accept error!");
			}
			return Socket(cli_fd);
		}

		 bool invalid()
		 {
			 return fd == INVALID_SOCKET;
		 }

		int send(const std::string& str)
		{
			if(!str.empty())
				return ::send(fd, str.data(), str.size(), 0);
			return 0;
		}

		int recv(char *buf,uint32_t len)
		{
			if (len > 0)
			{
				return ::recv(fd, buf, len, 0);
			}
			return 0;
		}
		~Socket()
		{
			if (fd != INVALID_SOCKET)
			{
				closesocket(fd);
			}
		}

	private:
		Socket(SOCKET fd_) : fd(fd_) {}
		Socket() noexcept(true) {
			fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		
		SOCKET fd;
	};

	
}



// TODO: 在此处引用程序需要的其他标头。
