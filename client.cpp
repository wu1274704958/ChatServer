#include "ChatServer.h"


int main(int argc, char* argv[])
{
	
	sock::WSAdata wsa_data(2, 2);

	
	//sock::Socket client = sock::Socket::client("47.94.232.85", 8888);
	sock::Socket client = sock::Socket::client("127.0.0.1", 8888);

	std::string sendData = "csi接收  二进制文件未解析\n";
	
	client.send(sendData);

	char recData[255] = {0};
	int ret = client.recv(recData, 255);
	if (ret > 0)
	{
		recData[ret] = 0x00;
		printf("%s",recData);
	}
	
	return 0;
}