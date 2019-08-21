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
#include "handler/RegHandler.h"
#include "handler/ServerStateHandler.h"

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
		users.push_back(User(10005, true, 22, SexType::Boy, "wws", "wws", "14253","",{}));
		users.push_back(User(10006, true, 22, SexType::Boy, "zff", "zff", "123456", "", {}));
		users.push_back(User(10007, true, 22, SexType::Boy, "hkp", "hkp", "123456", "", {}));
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

	wws::thread_pool pool(18);

	SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler, TRUE);

	std::atomic<bool> clear_task_running = true;

	std::function<void()> clear_task = [&clear_task_running,&clients]()
	{
		while (clear_task_running)
		{
			if (clients.size() > 0)
			{
				clients.change([](std::vector<std::shared_ptr<ab_client>>& cs) {
					for (auto it = cs.begin(); it != cs.end();)
					{
						if (!(*it)->is_invalid() && (*it)->last_heart_duration() > MaxHeartDuration)
						{
							try {
								(*it)->send_error<ErrorCode::OverTime>();
								(*it)->close();
							}
							catch (std::exception e) {
								dbg(e.what());
							}
							continue;
						}
						++it;
					}
				});
			}
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(30s);
		}
	};
	pool.add_task(clear_task);
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
				ac->set_uid(abc::INVALID_UID);
				ac->set_heart();
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
							case HandlerCode::Register:
							{
								RegHandler(users, clients, ac).handle(std::move(data_ptr));
								break;
							}
							case HandlerCode::Logout:
							{
								if (ac->get_client_type() == ClientType::Admin || ac->get_client_type() == ClientType::Default)
								{
									ac->set_client_type(ClientType::NotKnow);
									ac->set_uid(abc::INVALID_UID);
									ac->send_error<ErrorCode::Success>();
								}
								else {
									ac->send_error<ErrorCode::NotLogin>();
								}
								break;
							}
							case HandlerCode::NoHandler:
							{
								ac->send_error<ErrorCode::CanNotHandle>();
								break;
							}
							case HandlerCode::ServerState:
							{
								ServerStateHandler(users, clients, ac).handle(std::move(data_ptr));
								break;
							}
							case HandlerCode::Heart:
							{
								ac->set_heart();
								break;
							}
						}
					}
					catch (std::exception e)
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
	clear_task_running = false;
	while (pool.has_not_dispatched()) { Sleep(10); }
	pool.wait_all();
	std::cout << "Already shutdown the server!" << std::endl;
	::system("pause");
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
