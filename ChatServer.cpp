﻿// ChatServer.cpp: 定义应用程序的入口点。
//

#include "ChatServer.h"

using namespace std;

int main(int argc, char* argv[])
{
	//初始化WSA
	sock::WSAdata wsa_data(2, 2);

	sock::Socket ser = sock::Socket::server(8888, 5);

	
	struct sockaddr_in remoteAddr;
	
	while (1)
	{
		
		printf("等待连接...\n");
		try {
			sock::Socket cli = ser.accept(remoteAddr);
			printf("接受到一个连接：%s \r\n", inet_ntoa(remoteAddr.sin_addr));

			std::string ret = cli.recv(255);
			
			std::cout << ret;

			std::string sendData = "你好，TCP客户端！\n";
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

