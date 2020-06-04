#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "socket.h"

#pragma once
#include <WinSock2.h>
#include <Windows.h>

#ifdef max
#undef max
#endif // max

#ifdef min
#undef min
#endif // min

uint32_t sock::WSAdata::instance_count = 0;

namespace sock {

	union SockCastPtr
	{
		::SOCKET s;
		void* p;
	};

	template<typename S>
	void Socket::set_native_socket(S t)
	{
		SockCastPtr stop;
		stop.s = t;
		fd = stop.p;
	}

	template<typename S>
	S Socket::get_native_socket()
	{
		SockCastPtr stop;
		stop.p = fd;
		return stop.s;
	}

	
	::SOCKET to_s(void* p)
	{
		SockCastPtr stop;
		stop.p = p;
		return stop.s;
	}

	void* to_p(::SOCKET s)
	{
		SockCastPtr stop;
		stop.s = s;
		return stop.p;
	}

	Socket sock::Socket::server(uint16_t port, uint32_t backlog)
	{
		Socket self;

		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.S_un.S_addr = INADDR_ANY;
		if (bind(to_s( self.fd ), (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to bind!");
		}

		//¿ªÊ¼¼àÌý
		if (listen(to_s( self.fd ), backlog) == SOCKET_ERROR)
		{
			throw std::runtime_error("Listen error !");
		}
		self.set_addr(sin);
		return self;
	}

	Socket sock::Socket::client(const char* ip, uint16_t port) noexcept(false)
	{
		Socket self;

		struct sockaddr_in serAddr;
		serAddr.sin_family = AF_INET;
		serAddr.sin_port = htons(port);
		serAddr.sin_addr.S_un.S_addr = inet_addr(ip);

		if (connect(to_s( self.fd ), (struct sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
		{
			throw std::runtime_error("Failed to connect!");
		}
		self.set_addr(serAddr);
		return self;
	}

	Socket Socket::invalid() noexcept(true)
	{
		return Socket(to_p( INVALID_SOCKET));
	}

	template<typename Addr>
	void Socket::set_addr(Addr& addr)
	{
		ip = inet_ntoa(addr.sin_addr);
		if (wws::big_endian())
			port = addr.sin_port;
		else
			port = wws::reverse_byte(addr.sin_port);
	}

	WSAdata::WSAdata(uint32_t major, uint32_t version)  noexcept(false)
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
			return;
		}
		else
			init_success = true;
	}

	WSAdata::~WSAdata() {

		if (init_success)
		{
			--instance_count;
			WSACleanup();
		}
	}

	Socket::Socket(Socket&& oth)
	{
		fd = oth.fd;
		oth.set_native_socket(INVALID_SOCKET);
		ip = std::move(oth.ip);
		port = oth.port;
	}

	Socket& Socket::operator=(Socket&& oth) 
	{
		if (get_native_socket<::SOCKET>() != INVALID_SOCKET)
		{
			closesocket(get_native_socket<::SOCKET>());
		}
		this->fd = oth.fd;
		ip = std::move(oth.ip);
		port = oth.port;
		oth.set_native_socket( INVALID_SOCKET );
		return *this;
	}

	Socket Socket::accept()  noexcept(false)
	{
		sockaddr_in addr;
		int nAddrlen = static_cast<int>(sizeof(addr));
		SOCKET cli_fd = ::accept(get_native_socket<::SOCKET>(), (SOCKADDR*)&addr, &nAddrlen);
		if (cli_fd == INVALID_SOCKET)
		{
			throw std::runtime_error("Accept error!");
		}
		auto res = Socket(to_p(cli_fd));
		res.set_addr(addr);
		return res;
	}

	bool Socket::is_invalid()
	{
		return get_native_socket<::SOCKET>() == INVALID_SOCKET;
	}

	int Socket::send(const std::string& str)  noexcept(false)
	{
		if (!str.empty())
		{
			int ret = ::send(get_native_socket<::SOCKET>(), str.data(), static_cast<int>(str.size()), 0);
			if (ret == SOCKET_ERROR)
			{
				throw SocketCloseErr();
			}
			return ret;
		}
		return 0;
	}

	int Socket::send(char* buf, uint32_t len) noexcept(false)
	{
		if (buf)
		{
			int ret = ::send(get_native_socket<::SOCKET>(), buf, len, 0);
			if (ret == SOCKET_ERROR)
			{
				throw SocketCloseErr();
			}
			return ret;
		}
		return 0;
	}

	int Socket::recv(char* buf, uint32_t len) noexcept(true)
	{
		if (len > 0)
		{
			return ::recv(get_native_socket<::SOCKET>(), buf, len, 0);
		}
		return 0;
	}

	void Socket::close()
	{
		if (get_native_socket<::SOCKET>() != INVALID_SOCKET)
		{
			::closesocket(get_native_socket<::SOCKET>());
			set_native_socket(INVALID_SOCKET);
		}
	}

	Socket::~Socket()
	{
		if (get_native_socket<::SOCKET>() != INVALID_SOCKET)
		{
			::closesocket(get_native_socket<::SOCKET>());
		}
	}

	Socket::Socket() noexcept(true) {
		set_native_socket( socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) );
	}

	

}

template void sock::Socket::set_addr<::sockaddr_in>(::sockaddr_in&);
template void sock::Socket::set_native_socket<::SOCKET>(::SOCKET);
template ::SOCKET sock::Socket::get_native_socket<::SOCKET>();