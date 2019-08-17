// ChatServer.cpp: 定义应用程序的入口点。
//

#include "socket.h"
#include <string>
#include <dbg.hpp>
#include "tools/convert.h"
#include "ab_client.hpp"
#include "tools/thread_pool.h"
using namespace std;

int main(int argc, char* argv[])
{
	//初始化WSA
	sock::WSAdata wsa_data(2, 2);
	sock::Socket ser = sock::Socket::invalid();

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
	std::vector<std::shared_ptr<ab_client>> clients;
	wws::thread_pool pool(10);

	while (1)
	{
		printf("等待连接...\n");
		try {
			sock::Socket cli = ser.accept();
			std::cout << "接受到一个连接：" <<  cli.get_ip() << std::endl;

			int index = clients.size();
			clients.push_back(std::make_shared<ab_client>(cli));

			std::function<void()> f = [&clients,index]() {

				auto ac = clients[index];

				while (true)
				{
					try {
						auto[code, data_ptr] = ac->wait_request();
						switch (code)
						{
							case HandlerCode::Test:
							{
								int m = 0;
								if (!data_ptr)
								{
									ac->send_error<ErrorCode::ArgsError>();
									break;
								}

								m = data_ptr->get<int>("m");

								wws::Json data;
								data.put("result", m / 2);
								ac->send_error<ErrorCode::Success>(std::move(data));
								break;
							}
						}
					}
					catch (std::runtime_error e)
					{
						std::cerr << e.what() << std::endl;
						clients.erase(std::find(std::begin(clients), std::end(clients), ac));
						break;
					}
				}
			};

			pool.add_task(f);
			
		}
		catch (std::runtime_error e)
		{
			std::cout << e.what() << std::endl;
			continue;
		}
	}

	return 0;
}

