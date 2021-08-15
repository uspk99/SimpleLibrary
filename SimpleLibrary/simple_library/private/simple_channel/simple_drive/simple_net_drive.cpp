#include "../../../public/simple_channel/simple_net_drive.h"
#include "simple_net_drive_tcp.h"
#include "simple_net_drive_udp.h"


FSimpleNetDrive::FSimpleNetDrive()
	:MainConnection(nullptr)
{

}

FSimpleNetDrive* FSimpleNetDrive::GetNetDrive(ESimpleSocketType InSocketType, ESimpleDriveType InDriveType)//»ñÈ¡ÍøÂçÇý¶¯
{
	FSimpleNetDrive* NetDrive = nullptr;
	switch (InSocketType)
	{
	case ESimpleSocketType::SIMPLETYPE_TCP:
		NetDrive = new FSimpleTCPNetDrive(InDriveType);
		break;
	case ESimpleSocketType::SIMPLETYPE_UDP:
		NetDrive = new FSimpleUDPNetDrive(InDriveType);
		break;
	default:
		break;
	}

	return NetDrive;
}

bool FSimpleNetDrive::Init()
{
	return true;
}

void FSimpleNetDrive::Tick(double InTimeInterval)
{

}

FSimpleConnection* FSimpleNetDrive::GetFreeConnection()
{
	FSimpleConnection* FreeConnection = NULL;
	for (auto &Tmp:Connections)
	{
		if (Tmp.second->GetConnectionState()==ESimpleConnectionState::Free)
		{
			FreeConnection = Tmp.second;
			break;
		}
	}
	return FreeConnection;
}

void FSimpleNetDrive::SetNonblocking()
{

}
