#pragma once
#include "../../../public/simple_channel/simple_core/simple_channel.h"
#include "../../../public/simple_channel/simple_core/simple_connection.h"
#include "../../../public/simple_math/simple_math.h"

FSimpleChannel::FSimpleChannel()
	:Connection(nullptr)
	,ID(get_uint32_random(1000))
{

}

void FSimpleChannel::SetGuid(unsigned int InGuid)
{
	ID = InGuid;
}

void FSimpleChannel::Send(TArray<unsigned char>& InBuffer)
{
	Connection->SetBuffer(InBuffer);
	Connection->Send();
}

bool FSimpleChannel::Receive(TArray<unsigned char>& InBuffer)
{
	//bool bRecv = Connection->Recv();//是否网络接收成功
	Connection->RecvBuffer(InBuffer);
	return true;
}

void FSimpleChannel::SetConnection(FSimpleConnection* InConnection)
{
	Connection = InConnection;
}
