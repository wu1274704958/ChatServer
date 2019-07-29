#include "ChatServer.h"
#include <string>
#include <dbg.hpp>

int main(int argc, char* argv[])
{
	
	sock::WSAdata wsa_data(2, 2);

	sock::Socket client = sock::Socket::invalid();
	try
	{
		//sock::Socket client = sock::Socket::client("47.94.232.85", 8888);
		sock::Socket temp = sock::Socket::client("127.0.0.1", 8888);
		client = std::move(temp);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}

	std::string sendData = "csi接收  二进制文件未解析\n";

	client.send(sendData.size());
	client.send(sendData);

	int len = client.recv();
	dbg(len);
	std::string res = client.recv(len);
	std::cout << res;

	return 0;
	
}