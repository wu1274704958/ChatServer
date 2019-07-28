#include "ChatServer.h"


int main(int argc, char* argv[])
{
	
	sock::WSAdata wsa_data(2, 2);

	
	//sock::Socket client = sock::Socket::client("47.94.232.85", 8888);
	sock::Socket client = sock::Socket::client("127.0.0.1", 8888);

	std::string sendData = "csi接收  二进制文件未解析\n";
	
	client.send(sendData);

	std::string res  = client.recv(255);
	std::cout << res;
	return 0;
}