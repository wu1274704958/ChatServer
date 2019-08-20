// ChatServer.cpp: 定义应用程序的入口点。
//

#include "socket.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include "ab_client.hpp"
#include "tools/thread_pool.h"
#include "ab_clients.h"
#include "forms/User.h"
#include "tools/form.h"
#include "handler/LoginHandler.h"

using namespace std;

std::atomic<bool> running = true;
bool consoleHandler(int signal);
sock::Socket ser = sock::Socket::invalid();

int main(int argc, char* argv[])
{
	using namespace forms;
	using namespace handler;
	wws::form<User> users("User");

	
	if (users.empty())
	{
		users.push_back(User(10005, true, 22, "wws", "wws", "123456", {}));
	}
	

	//初始化WSA
	sock::WSAdata wsa_data(2, 2);

	try {
		sock::Socket temp = sock::Socket::server(8888, 5);
		ser = std::move(temp);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what() << std::endl;
		return -1;
	}
	using namespace abc;
	
	ab_clients clients;

	wws::thread_pool pool(10);

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler, TRUE);

	while (running)
	{
		printf("等待连接...\n");
		try {
			sock::Socket cli = ser.accept();
			std::cout << "接受到一个连接：" <<  cli.get_ip() << std::endl;

			std::shared_ptr<ab_client> ptr = std::make_shared<ab_client>(cli);
			clients.push_back(ptr);
			
			std::function<void()> f = [&users,&clients,ac = std::move(ptr)]() mutable {
				ac->set_client_type(ClientType::NotKnow);
				while (true)
				{
					try {
						auto[code, data_ptr] = ac->wait_request();
						switch (code)
						{
							case HandlerCode::Test:
							{
								double m = 0.0;
								if (!data_ptr)
								{
									ac->send_error<ErrorCode::ArgsError>();
									break;
								}

								m = static_cast<double>(data_ptr->get<int>("m"));

								wws::Json data;
								data.put("result", m / 2.0);
								ac->send_error<ErrorCode::Success>(std::move(data));
								break;
							}
							case HandlerCode::Login:
							{
								LoginHandler(users,clients,ac).handle(std::move(data_ptr));
								break;
							}
						}
					}
					catch (std::runtime_error e)
					{
						dbg(e.what());
						clients.erase(ac);
						dbg(clients.size());
						break;
					}
				}
			};

			pool.add_task(f);
			
		}
		catch (std::runtime_error e)
		{
			dbg(e.what());
			continue;
		}
	}

	while (pool.has_not_dispatched()) { Sleep(10); }
	pool.wait_all();
	std::cout << "Already shutdown the server!" << std::endl;
	system("pause");
	return 0;
}


bool consoleHandler(int signal) {

	if (signal == CTRL_C_EVENT) {
		
		running = false;
		if (!ser.is_invalid())
		{
			ser.close();
		}
	}
	return true;
}
