#include "socket.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include <fstream>
#include "forms/User.h"
#include "tools/form.h"
#include "tools/thread_pool.h"
#include "json.hpp"
#include <cstdlib>

void test_Login(sock::Socket&);
void test_Reg(sock::Socket&);
void test_m_thread(bool one = true);
void test_Test(sock::Socket&);
void ServerState(sock::Socket&);
void Logout(sock::Socket&);
sock::Socket link_server(bool Local = true);
bool send(sock::Socket& cli,std::string& data);
std::string recv(sock::Socket& cli);

int main(int argc, char* argv[])
{
	int local;
	std::cout << "1.local\n2.remote\n";

	std::cin >> local;
	if (local == 1 || local == 2)
	{

		sock::WSAdata wsa_data(2, 2);

		sock::Socket client = link_server( local == 1 );

		if (client.is_invalid())
		{
			system("pause");
			return -1;
		}
		int c = -1;
		
		bool running = true;
		while (running)
		{
			std::cout << "1.Test\n2.Reg\n3.Login\n4.ServerState\n5.Logout\n0.exit\n";
			std::cin >> c;
			switch (c)
			{
			case 0:
				running = false;
				break;
			case 1:
				test_Test(client);
				break;
			case 2:
				test_Reg(client);
				break;
			case 3:
				test_Login(client);
				break;
			case 4:
				ServerState(client);
				break;
			case 5:
				Logout(client);
			default:
				break;
			}
		}

	}
	system("pause");
	return 0;	
}


void test_m_thread(bool one)
{
	std::function<void()> f1 = []() {
		for (int i = 0; i < 200; ++i)
		{
			std::cout << i;
			Sleep(20);
		}
		std::cout << "\n";
		std::cout << "f1 end\n";
	};

	int b = 200;
	std::function<void()> f2 = [b]() {
		for (int i = b; i < 200 + b; ++i)
		{
			std::cout << i;
			Sleep(20);
			
		}
		std::cout << "\n";
		std::cout << "f2 end\n";
	};

	int c = 400;
	std::function<void()> f3 = [c]() {
		for (int i = c; i < 200 + c; ++i)
		{
			std::cout << i;
			Sleep(20);

		}
		std::cout << "\n";
		std::cout << "f3 end\n";
	};
	if (one)
	{
		wws::m_thread th(f1);

		while (!th.can_set_task()) {}
		th.set_task(f2);
		while (!th.can_set_task()) {}
		th.set_task(f3);
		th.wait_and_stop();
	}
	else {
		wws::thread_pool pool(2);

		pool.add_task(f1);
		pool.add_task(f2);
		pool.add_task(f3);
		pool.add_task([]() { std::cout << "\nhello!!!"; });

		while (pool.has_not_dispatched()) { Sleep(5); }
		dbg("dispatcher all!");
		pool.wait_all();
		//system("pause");
	}

}

void test_Test(sock::Socket& client)
{
	std::srand(std::time(nullptr));

	wws::Json req;
	req.put("reqn", "Test");

	wws::Json data;
	data.put("m", std::rand() % 200 + 1);

	req.put("data", std::move(data));

	std::string sendData = req.to_string();

	dbg(sendData);
	int len;
	
	if (!send(client, sendData))
	{
		return;
	}

	dbg(len);
	std::string res;
	try {
		res = recv(client);
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	std::string res_utf8 = dbg(cvt::utf8_l(res));

	wws::Json resj(res_utf8);
	try {
		dbg(resj.get<int>("ret"));
		if (resj.has_key("data"))
			dbg(resj.get_obj("data").get<double>("result"));
		}
	catch (std::exception e)
	{
		dbg(e.what());
	}
}

void test_Login(sock::Socket& client)
{
	wws::Json req;
	req.put("reqn", "Login");

	std::string acc;
	std::string psd;

	std::cout << "acc:";
	std::cin >> acc;

	std::cout << "psd:";
	std::cin >> psd;

	wws::Json data;
	data.put("acc", acc);
	data.put("psd", psd);

	req.put("data", std::move(data));

	std::string sendData = req.to_string();

	dbg(sendData);
	int len;

	if (!send(client, sendData))
	{
		return;
	}

	dbg(len);
	std::string res;
	try {
		res = recv(client);
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	std::string res_utf8 = dbg(cvt::utf8_l(res));

	wws::Json resj(res_utf8);
	try {
		dbg(resj.get<int>("ret"));
	}
	catch (std::exception e)
	{
		dbg(e.what());
	}

}

void test_Reg(sock::Socket& client)
{

	wws::Json req;
	req.put("reqn", "Register");

	std::string acc;
	std::string psd;
	std::string name;
	int age;
	char sex;

	std::cout << "acc:";
	std::cin >> acc;

	std::cout << "psd:";
	std::cin >> psd;

	std::cout << "age:";
	std::cin >> age;

	std::cout << "sex:";
	std::cin >> sex;

	std::cout << "name:";
	std::cin >> name;

	wws::Json data;
	data.put("acc", acc);
	data.put("psd", psd);
	data.put("sex", sex);
	data.put("age", age);
	data.put("name", name);


	req.put("data", std::move(data));

	std::string sendData = req.to_string();

	dbg(sendData);

	int len;

	if (!send(client, sendData))
	{
		return;
	}

	dbg(len);
	std::string res;
	try {
		res = recv(client);
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	std::string res_utf8 = dbg(cvt::utf8_l(res));

	wws::Json resj(res_utf8);

	/*dbg(resj.get<int>("ret"));
	if (resj.has_key("data"))
		dbg(resj.get_obj("data").get<double>("result"));*/

}

void ServerState(sock::Socket& client)
{
	wws::Json req;
	req.put("reqn", "ServerState");
	
	std::string sendData = req.to_string();

	dbg(sendData);

	int len;

	if (!send(client, sendData))
	{
		return;
	}

	std::string res;
	try {
		res = recv(client);
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	dbg(res);

	try {
		wws::Json rj(res);
		auto data = rj.detach_obj("data");
		std::cout << "-------------------------------------------\n";
		std::cout << "在线id:\t" <<	data.get_str("olusers") << '\n';
		std::cout << "在线ip:\t" <<	data.get_str("addrs") << '\n';
		std::cout << "管理员:\t" <<	data.get_str("admin") << " 人\n";
		std::cout << "普通:\t" <<	data.get_str("ordinary") << " 人\n";
		std::cout << "闲置:\t" <<	data.get_str("Idler") << " 人\n";
		std::cout << "-------------------------------------------\n";
	}
	catch (std::exception e)
	{
		dbg(e.what());
		return;
	}
}

void Logout(sock::Socket& client)
{
	wws::Json req;
	req.put("reqn", "Logout");

	std::string sendData = req.to_string();

	dbg(sendData);

	int len;

	if (!send(client, sendData))
	{
		return;
	}

	std::string res;
	try {
		res = recv(client);
	}
	catch (std::exception e)
	{
		std::cout << e.what() << std::endl;
		return;
	}
	dbg(res);
}

sock::Socket link_server(bool Local)
{
	sock::Socket client = sock::Socket::invalid();
	try
	{
		if (Local)
			client = sock::Socket::client("127.0.0.1", 8888);
		else
			client = sock::Socket::client("47.94.232.85", 8888);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
	}
	return client;
}

bool send(sock::Socket& cli,std::string& data)
{
	try {
		cli.send<char>(0x07);
		cli.send<int>(data.size());
		cli.send(data);
		cli.send<int>(0x0);
		cli.send<char>(0x09);
	}
	catch (std::exception e)
	{
		dbg(e.what());
		return false;
	}
	return true;
}

std::string recv(sock::Socket& cli)
{
	while (cli.recv<unsigned char>() != 0x07) {}

	int len = cli.recv<int>();
	std::string req = cli.recv(len);
	int CRC = cli.recv<int>();
	if (cli.recv<unsigned char>() != 0x09)
	{
		throw std::exception("bad pkg");
	}
	return req;
}

