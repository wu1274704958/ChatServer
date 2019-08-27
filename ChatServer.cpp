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
#include "handler/LoginHandler.h"
#include "handler/RegHandler.h"
#include "handler/ServerStateHandler.h"
#include "handler/ModifyHandler.h"
#include <sqlpp/Drive.h>
#include <sqlpp/Connect.h>
#include <sqlpp/Query.hpp>
#include <sqlpp/Result.hpp>


using namespace std;

std::atomic<bool> running = true;
bool consoleHandler(int signal);
sock::Socket ser = sock::Socket::invalid();

int main(int argc, char* argv[])
{
	using namespace forms;
	using namespace handler;
	using namespace sql;

	Drive dri;
	Connect conn = dri.connect("localhost", "root", "As147258369", "cs", 3306);

	Query q;

	try {

		Result res = q.select(&User::uid)
			.where<K::eq, false>(&User::uid, "10005")
			.exec(conn);
		q.clear();

		if (res.rows() == 0)
		{
			User u1(10005, true, 22, SexType::Boy, "wws", "wws", "14253", "", {});
			u1.set_reg_time_now();
			q.insert(u1, &User::uid, &User::is_admin,  &User::sex, &User::age, &User::name,&User::acc, &User::psd, &User::head, &User::friends,&User::reg_time)
				.exec(conn);
			User u2(10006, true, 22, SexType::Boy, "zff", "zff", "123456", "", {});
			u2.set_reg_time_now();
			User u3(10007, true, 22, SexType::Boy, "hkp", "hkp", "123456", "", {});
			u3.set_reg_time_now();
			User u4(10008, true, 22, SexType::Boy, "zck", "zck", "123456", "", {});
			u4.set_reg_time_now();
			q.clear();
			q.insert(u2, &User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends, &User::reg_time)
				.exec(conn);
			q.clear();
			q.insert(u3, &User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends, &User::reg_time)
				.exec(conn);
			q.clear();
			q.insert(u4, &User::uid, &User::is_admin, &User::sex, &User::age, &User::name, &User::acc, &User::psd, &User::head, &User::friends, &User::reg_time)
				.exec(conn);
		}
	}
	catch (SqlException e)
	{
		dbg(e.what());
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
						if (!(*it)->is_invalid() && !(*it)->is_busy() && (*it)->last_heart_duration() > MaxHeartDuration)
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
			
			std::function<void()> f = [&conn,&clients,ac = std::move(ptr)]() mutable {
				ac->set_client_type(ClientType::NotKnow);
				ac->set_uid(abc::INVALID_UID);
				ac->set_heart();
				while (true)
				{
					try {
						auto[code, data_ptr] = ac->wait_request();
						auto bg = ac->get_busy_guard();
						switch (code)
						{
							case HandlerCode::Test:
							{
								double m = 0.0;
								if (!data_ptr)
								{
									ac->send_error<ErrorCode::ArgsError, HandlerCode::Test>();
									break;
								}

								m = static_cast<double>(data_ptr->get<int>("m"));

								wws::Json data;
								data.put("result", m / 2.0);
								ac->send_error<ErrorCode::Success,HandlerCode::Test>(std::move(data));
								break;
							}
							case HandlerCode::Login:
							{
								LoginHandler(conn,clients,ac).handle(std::move(data_ptr));
								break;
							}
							case HandlerCode::Register:
							{
								RegHandler(conn, clients, ac).handle(std::move(data_ptr));
								break;
							}
							case HandlerCode::Logout:
							{
								if (ac->get_client_type() == ClientType::Admin || ac->get_client_type() == ClientType::Default)
								{
									ac->set_client_type(ClientType::NotKnow);
									ac->set_uid(abc::INVALID_UID);
									ac->send_error<ErrorCode::Success, HandlerCode::Logout>();
								}
								else {
									ac->send_error<ErrorCode::NotLogin, HandlerCode::Logout>();
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
								ServerStateHandler(conn, clients, ac).handle(std::move(data_ptr));
								break;
							}
							case HandlerCode::Heart:
							{
								ac->set_heart();
								ac->send_error<ErrorCode::Success, HandlerCode::Heart>();
								break;
							}
							case HandlerCode::ModifyInfo:
							{
								ModifyInfo(std::make_tuple(
									std::ref(clients),
									ac,
									std::ref(conn)
								)).handle(std::move(data_ptr));
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
