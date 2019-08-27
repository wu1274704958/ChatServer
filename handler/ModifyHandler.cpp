#include "ModifyHandler.h"

namespace handler {

	void ModifyInfo::handle(std::shared_ptr<wws::Json>&& data)
	{
		auto[clients,client,conn] = get_args();
		this->error<abc::ErrorCode::UnImplement>(*client);
	}

}
