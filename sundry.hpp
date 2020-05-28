#pragma once
#include <zlib.h>
#include <fileop.hpp>
#include <string>
#include <filesystem>
#include <optional>

namespace sundry {
	template<size_t S,size_t MBS = 1024 * 20,size_t UBS = 1024 * 10>
	std::optional<std::string> uncompress_from_file(std::filesystem::path& f)
	{
		auto [op,len] = wws::read_from_file_bin<S>(f, std::ios::binary);
		if (op)
		{
			auto src = op.get();
			uLongf curr_size = MBS + 1;
			Bytef* ds = reinterpret_cast<Bytef*>(std::malloc(MBS + 1));
			uLongf src_len = len;
			if (!ds) return {};
			std::memset(ds, 0, curr_size);
			int ret;
			while ((ret = uncompress(ds, &curr_size, 
				reinterpret_cast<const unsigned char*>(src), src_len)) == Z_BUF_ERROR)
			{
				auto nds = reinterpret_cast<Bytef*>(std::realloc(ds, curr_size + UBS));
				if (nds) 
				{ 
					ds = nds;
					curr_size += UBS;
					std::memset(ds, 0, curr_size);
				}
				else
				{
					free(ds);
					return {};
				}
			}
			if (ret == Z_OK)
			{
				auto res = std::make_optional(std::string(reinterpret_cast<const char*>(ds)));
				free(ds);
				return res;
			}
			else
			{
				free(ds);
				return {};
			}
				
		}else{
			return {};
		}
	}
	template<size_t S>
	bool compress_to_file(std::filesystem::path& f,std::string s)
	{
		auto len = compressBound(s.size() + 1);
		unsigned char* data = new unsigned char[len];
		std::memset(data, 0, len);
		if (!data) return false;
		if(Z_OK == compress(reinterpret_cast<unsigned char*>(data), &len, reinterpret_cast<const unsigned char*>(s.c_str()), s.size() + 1))
		{
			auto res = wws::write_to_file_bin<S>(f, reinterpret_cast<const unsigned char*>(data),len, std::ios::binary);
			delete[] data;
			return res;
		}else{
			delete[] data;
			return false;
		}
	}
}