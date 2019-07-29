#include "ChatServer.h"
#include <string>
#include <dbg.hpp>

int main(int argc, char* argv[])
{
	
	sock::WSAdata wsa_data(2, 2);

	
	//sock::Socket client = sock::Socket::client("47.94.232.85", 8888);
	sock::Socket client = sock::Socket::client("127.0.0.1", 8888);

	std::string sendData = "csi接收  二进制文件未解析\n";
	
	client.send(sendData.size());
	client.send(sendData);

	int len = client.recv();
	dbg(len);
	std::string res  = client.recv(len);
	std::cout << res;
	return 0;
}