#include "..\sundry.hpp"
#include "UploadVerifyKV.h"
#include <fileop.hpp>
#include <array>
#include <filesystem>



using namespace abc;
using namespace wws;
namespace fs = std::filesystem;

std::string path("./VerifyKV.json");

void handler::UploadVerifyKV::handle(std::shared_ptr<wws::Json>&& data)
{
	auto [clients, client, conn] = get_args();
	if (client->get_client_type() != ClientType::Admin)
	{
		error<ErrorCode::PermissionDenied>(client);
		return;
	}
	if (!data || !(data->has_key("key") && data->has_key("val")))
	{
		error<ErrorCode::ArgsError>(client);
		return;
	}
	std::string key = data->get<std::string>("key"), val = data->get<std::string>("val");

	std::string verify("{}");
	fs::path f(path);
	if (fs::exists(f))
	{
		auto op = sundry::uncompress_from_file<1024>(f);
		if (op)
		{
			verify = op.value();
		}
	}
	try {
		wws::Json v(verify);
		v.put(key, val);
		bool ret = sundry::compress_to_file<1024>(f, v.to_string());
		if (ret)
			error<ErrorCode::Success>(client);
		else
			error<ErrorCode::Failed>(client);
	}
	catch (std::exception& e)
	{
		wws::Json d;
		d.put("err_msg", e.what());
		error<ErrorCode::Failed>(client,std::move(d));
	}
	return;
}


void handler::DownloadVerifyKV::handle(std::shared_ptr<wws::Json>&& data)
{
	auto [clients, client, conn] = get_args();
	
	if (!data || !data->has_key("key"))
	{
		error<ErrorCode::ArgsError>(client);
		return;
	}
	std::string key = data->get<std::string>("key");

	std::string verify("{}");
	fs::path f(path);
	wws::Json d;
	if (fs::exists(f))
	{
		auto op = sundry::uncompress_from_file<1024,1024,2>(f);
		if (op)
		{
			verify = op.value();
			dbg(verify);
		}
	}
	else {
		d.put("err_msg", "Not this key!!!");
		error<ErrorCode::Failed>(client, std::move(d));
		return;
	}
	try {
		wws::Json v(verify);
		if(!v.has_key(key))
		{
			d.put("err_msg", "Not this key!!!");
			error<ErrorCode::Failed>(client, std::move(d));
			return;
		}
		
		d.put("val",v.get<std::string>(key));
		
		error<ErrorCode::Success>(client,std::move(d));
	}
	catch (std::exception& e)
	{
		wws::Json d;
		d.put("err_msg", e.what());
		error<ErrorCode::Failed>(client, std::move(d));
	}
	return;
}
