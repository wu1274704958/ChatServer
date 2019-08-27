#pragma once

namespace abc {

	enum class ErrorCode : int {
		Success = 0,
		ArgsError = -1,
		BadRequest = -2,
		CanNotHandle = -3,
		IncorrectPassword = -4,
		IncorrectAccount = -5,
		PermissionDenied = -6,
		AlreadyLogged = -7,
		Failed = -8,
		AlreadyRegister = -9,
		NotLogin = -10,
		TooBigDataPkg = -11,
		BadDataPkg = -12,
		OverTime = -13,
		UnKnowErr = -14,
		UnImplement = -15
	};

	enum class HandlerCode : unsigned int
	{
		NoHandler = 0x80000000,
		Invaild = 0x7fffffff,
		Register = 0,
		Test = 1,
		Login = 2,
		Logout = 3,
		ServerState = 4,
		Heart = 5,
		ModifyInfo = 6
	};

	enum class ClientType : int
	{
		NotKnow = -1,
		Default = 0,
		Upload = 1,
		Download = 2,
		P2P = 3,
		Admin = 4
	};

	constexpr int MAX_BYTE_SIZE = 1024 * 1024 * 2;
	constexpr int INVALID_UID = -1;
	constexpr unsigned char BeginBit = 0x07;
	constexpr unsigned char EndBit = 0x09;
	constexpr long long MaxHeartDuration = 6000 * 10;
}