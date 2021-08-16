#pragma once
#include "../simple_core_minimal/simple_c_core/simple_core_minimal.h"

enum class ESimpleSocketType :unsigned char
{
	SIMPLETYPE_UDP,
	SIMPLETYPE_TCP,
};

enum class ESimpleDriveType :unsigned char
{
	DRIVETYPE_LISTEN,//服务器 监听
	DRIVETYPE_CONNECTION,//客户端 连接
};

enum class ESimpleConnectionState :unsigned char
{
	Free,
	Version_Verfication,//版本校验
	Login,
	Join,
};

struct FSimpleIoData
{
	FSimpleIoData();


	OVERLAPPED Overlapped;
	CHAR Buffer[1024];
	BYTE Type;//行为: 读取 写入
	DWORD Len;
	WSABUF WsaBuffer;
};

struct FSimpleBunchHead//数据头
{
	FSimpleBunchHead();
	
	unsigned int Protocols;//协议号
	unsigned int ChannelID;//通道ID
	unsigned int ParamNum;//参数数量
	unsigned int ParamLength;
};