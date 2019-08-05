#include "ChatServer.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include <fstream>
#include "forms/User.h"
#include "tools/form.h"

int main(int argc, char* argv[])
{
	using namespace forms;

	wws::form<User> users("User");
	
	users.change([](std::vector<User>& its)
	{
		if (its.empty())
		{
			User u(89, 17, "sss", "1274704958", "hjxvags", std::vector<uint32_t>());
			User u2(90, 17, "sss2", "wu1274704958", "hjxvags", std::vector<uint32_t>());

			u2.friends.push_back(89);
			u2.friends.push_back(100);

			its.push_back(std::move(u));
			its.push_back(std::move(u2));
		}
		else {
			
			for (auto& u : its)
			{
				std::cout << u;
			}
			
		}
	});
	
	sock::WSAdata wsa_data(2, 2);

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

	std::string sendData = u8"csi接收  二进制文件未解析\n";

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
	std::cout << cvt::utf8_l(res);
	return 0;
	
}