#pragma once
#include "../../../public/simple_channel/simple_net_drive.h"
#include "../../../public/simple_channel/simple_core/simple_connection.h"


class FSimpleTCPNetDrive :public FSimpleNetDrive
{
	typedef FSimpleNetDrive Super;
public:
	FSimpleTCPNetDrive(ESimpleDriveType InDriveType);

	virtual bool Init();
	virtual void Tick(double InTimeInterval);
	
	static HANDLE CompletionPortHandle;
protected:
	
	ESimpleDriveType DriveType;
	HANDLE hThreadHandle[32];
	WSADATA WsaData;
	virtual void SetNonblocking();//…Ë÷√∑«◊Ë»˚
};