#pragma once
#include "../../../public/simple_channel/simple_core/simple_connection.h"
#include "public/simple_channel/simple_core/simple_channel.h"

FSimpleConnection::FSimpleConnection()
	:ConnectionState(ESimpleConnectionState::Free)
	,DriverType(ESimpleDriveType::DRIVETYPE_LISTEN)
	,DataLength(0)
{
	
}

bool FSimpleConnection::Init()
{
	//分配通道
	for (int i = 0; i < 10; i++)
	{
		//反向注册
		Channels.emplace_back(FSimpleChannel());
		FSimpleChannel &Inst = Channels.back();
		Inst.SetConnection(this);
	}
	return false;
}

BOOL FSimpleConnection::Recv()
{
	IOData.Type = 0;//接收0 发送1
	IOData.WsaBuffer.buf = IOData.Buffer;
	IOData.WsaBuffer.len = 1024;
	if (DriverType==ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		DWORD Flag = 0;
		DWORD Len = 0;
		int RecvCount = WSARecv(Socket, &IOData.WsaBuffer, 1,//WSA结构的数量
			&Len,//提取长度
			&Flag,
			&IOData.Overlapped,
			NULL);
		if (RecvCount == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)//ERROR_IO_PENDING 数据尚未发送完毕  
				//GetOverlappedResult 查询结果
			{
				return FALSE;
			}
		}
	}
	else
	{
		int Flag = 0;
		int Len = 1024;
		int RecvCount = recv(Socket, IOData.WsaBuffer.buf, Len, Flag);
#if 0
		int count = 4;
		while (count > 0)
		{
			int leng = recv(Socket, (char*)&DataLength, count, Flag);
			if (leng == -1)
			{
				DataLength = -1;
			}
			else if (leng == 0)
			{
				DataLength = 4 - count;
			}

		}
#endif



		if (RecvCount ==SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)\
			{
				return FALSE;
			}
		}
		else
		{
			IOData.WsaBuffer.len = RecvCount;
		}
	}
	return TRUE;
}

BOOL FSimpleConnection::Send()
{
	DWORD Flag = 0L;
	IOData.Type = 1;
	IOData.WsaBuffer.buf = IOData.Buffer;
	//strlen以\0结尾，如果buffer前面有\0就会不准确
	//IOData.WsaBuffer.len = strlen(IOData.Buffer);
	IOData.WsaBuffer.len = IOData.Len;

	int count = IOData.Len;
	const char* BufTest = IOData.WsaBuffer.buf;
	while (count>0)
	{	
		int DataLen = send(Socket, BufTest, count,Flag);
		if (DataLen==SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)//GetOverlappedResult
			{
				return FALSE;
			}
		}
		else if (DataLen==0)
		{
			continue;
		}
		BufTest += DataLen;
		count -= DataLen;

	}
#if 0
	if (IOData.WsaBuffer.len > 0)
	{
		if (send(Socket,
			IOData.WsaBuffer.buf,
			IOData.WsaBuffer.len,
			Flag) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)//GetOverlappedResult
			{
				return FALSE;
			}
		}
	}
#endif


	return TRUE;
}

void FSimpleConnection::SetBuffer(TArray<unsigned char>& InBuffer)
{
	//将TArray中的数据传入IOData
	void* InData = InBuffer.GetData();
	IOData.Len = InBuffer.Num();
	memset(IOData.Buffer, 0, sizeof(IOData.Buffer));

	//增加4字节的包头防止粘包
#if 0
	int bigLen = htonl(IOData.Len);
	IOData.Len += 4;
	memcpy(IOData.Buffer, (char*)&bigLen, 4);
	memcpy(IOData.Buffer + 4, InData, InBuffer.Num());

#else
	memcpy(IOData.Buffer,InData,InBuffer.Num());
#endif



}

void FSimpleConnection::RecvBuffer(TArray<unsigned char>& InBuffer)
{
	int DataLen = 0;
	//auto Msg = InBuffer;	
	//接收IOData的数据到TArray
	int count = 4;
	//DataLen = ntohl(recv(Socket, (char*)&DataLen, count, 0));

	//if (FSimpleBunchHead* Head = (FSimpleBunchHead*)(IOData.Buffer+4))
	if (FSimpleBunchHead* Head = (FSimpleBunchHead*)(IOData.Buffer))
	{
		if (Head->ParamNum>0)
		{
			InBuffer.AddUninitialized(IOData.Len);
			void* InData = InBuffer.GetData();//获取首地址
			memcpy(InData, IOData.Buffer,InBuffer.Num());
		}
	}
	memset(IOData.Buffer, 0, InBuffer.Num());
}

void FSimpleConnection::SetConnectionState(const ESimpleConnectionState& InNewConnectionState)
{
	ConnectionState = InNewConnectionState;
}

FSimpleChannel* FSimpleConnection::GetMainChannnel()
{
	return &(*Channels.begin());
}

FSimpleChannel* FSimpleConnection::GetChannnel(int InID)
{
	for (auto &Tmp:Channels)
	{
		if (Tmp.GetGuid() == InID)
		{
			return &Tmp;
		}
	}
	return nullptr;
}

void FSimpleConnection::SetDriveType(const ESimpleDriveType InDriveType)
{
	DriverType = InDriveType;
}

void FSimpleConnection::GetChannnelActiveID(std::vector<int>& InIDs)
{
	for (auto& Tmp : Channels)
	{
		InIDs.push_back(Tmp.GetGuid());
	}
}

//127.0.0.1:7777
std::string FSimpleConnection::GetAddString()
{
	std::string StringAddr = inet_ntoa(ConnectAddr.sin_addr);

	char Buff[32] = { 0 };
	_itoa_s(ConnectAddr.sin_port, Buff, 32, 10);//10进制

	return StringAddr + ":" + Buff;
}

std::list<FSimpleChannel>* FSimpleConnection::GetChannels()
{
	return &Channels;
}
