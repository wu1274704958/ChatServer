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

using namespace std;

std::atomic<bool> running = true;
bool consoleHandler(int signal);
sock::Socket ser = sock::Socket::invalid();

int main(int argc, char* argv[])
{
	using namespace forms;
	wws::form<User> users("User");

	users.change([](std::vector<User>& us) {
		if (us.empty())
		{
			us.push_back(User(10005, true, 22, "wws", "wws", "123456", {}));
		}
	});

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
								if (!data_ptr)
								{
									ac->send_error<ErrorCode::ArgsError>();
									break;
								}
								else {
									std::string acc;
									std::string psd;
									try {
										acc = data_ptr->get<std::string>("acc");
										psd = data_ptr->get<std::string>("psd");

										users.change([ac,acc = std::move(acc),psd = std::move(psd)](std::vector<forms::User>& us) mutable {
											bool has = false;
											bool right_psd = false;

											for (auto& u : us)
											{
												if (u.acc == acc)
												{
													has = true;
													right_psd = u.psd == psd;
													if (right_psd)
													{
														ac->set_client_type(ClientType::Default);

														wws::Json dat = wws::toJson(u,
															{ "id","acc","psd","age","is_admin","name","friends" },
															&User::id, &User::acc, &User::psd, &User::age, &User::is_admin, &User::name, &User::friends);
														ac->send_error<ErrorCode::Success>(std::move(dat));
													}
													else {
														ac->send_error<ErrorCode::IncorrectPassword>();
													}
													break;
												}
											}
											if (!has)
												ac->send_error<ErrorCode::IncorrectAccount>();
										});
									}
									catch (wws::BadKeyErr e)
									{
										ac->send_error<ErrorCode::ArgsError>();
										break;
									}
								}
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
