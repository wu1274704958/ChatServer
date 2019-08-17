﻿// ChatServer.cpp: 定义应用程序的入口点。
//

#include "socket.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include "ab_client.hpp"
using namespace std;

int main(int argc, char* argv[])
{
	//初始化WSA
	sock::WSAdata wsa_data(2, 2);
	sock::Socket ser = sock::Socket::invalid();

	try {
		sock::Socket temp = sock::Socket::server(8888, 5);
		ser = std::move(temp);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	while (1)
	{
		
		printf("等待连接...\n");
		try {
			sock::Socket cli = ser.accept();
			std::cout << "接受到一个连接：" <<  cli.get_ip() << std::endl;

			int len = cli.recv<int>();
			dbg(len);
			std::string ret = cli.recv(len);
			
			std::cout << cvt::utf8_l(ret);

			std::string temp = "你好，TCP客户端！\n";
			std::string sendData = cvt::l_utf8(temp);
			dbg(sendData.size());
			cli.send(sendData.size());
			cli.send(sendData);
		}
		catch (std::runtime_error e)
		{
			std::cout << e.what() << std::endl;
			continue;
		}
	}

	return 0;
}

