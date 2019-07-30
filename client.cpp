#include "ChatServer.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include <fstream>

int main(int argc, char* argv[])
{
	
	sock::WSAdata wsa_data(2, 2);

	std::string uc = u8"哈哈!";

	std::string l = cvt::utf8_l(uc);
	dbg(l.size());
	dbg(l);

	std::string utf = cvt::l_utf8(l);

	std::ofstream of("gbk.txt");
	of << l;
	
	std::ofstream of2("utf.txt");
	of2 << utf;

	sock::Socket client = sock::Socket::invalid();
	try
	{
		sock::Socket temp = sock::Socket::client("47.94.232.85", 8888);
		//sock::Socket temp = sock::Socket::client("127.0.0.1", 8888);
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
	int len;
	try {
		len = client.recv<int>();
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	
	dbg(len);
	std::string res;
	try {
		res = client.recv(len);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	std::cout << res;
	return 0;
	
}