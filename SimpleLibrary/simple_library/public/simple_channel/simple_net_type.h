#pragma once
#include "../simple_core_minimal/simple_c_core/simple_core_minimal.h"

enum class ESimpleSocketType :unsigned char
{
	SIMPLETYPE_UDP,
	SIMPLETYPE_TCP,
};

enum class ESimpleDriveType :unsigned char
{
	DRIVETYPE_LISTEN,//������ ����
	DRIVETYPE_CONNECTION,//�ͻ��� ����
};

enum class ESimpleConnectionState :unsigned char
{
	Free,
	Version_Verfication,//�汾У��
	Login,
	Join,
};

struct FSimpleIoData
{
	FSimpleIoData();


	OVERLAPPED Overlapped;
	CHAR Buffer[1024];
	BYTE Type;//��Ϊ: ��ȡ д��
	DWORD Len;
	WSABUF WsaBuffer;
};

struct FSimpleBunchHead//����ͷ
{
	FSimpleBunchHead();
	
	unsigned int Protocols;//Э���
	unsigned int ChannelID;//ͨ��ID
	unsigned int ParamNum;//��������
	unsigned int ParamLength;
};