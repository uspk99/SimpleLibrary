#pragma once

#include "simple_core/simple_connection.h"
#include "simple_net_type.h"
#include "../simple_cpp_core_minimal/simple_cpp_core_minimal.h"

class FSimpleNetDrive
{
public:
	FSimpleNetDrive();

	static FSimpleNetDrive* GetNetDrive(ESimpleSocketType InSocketType, ESimpleDriveType InDriveType);//获取网络驱动

	virtual bool Init();

	virtual void Tick(double InTimeInterval);
protected:
	FSimpleConnection* MainConnection;
	std::map<int, FSimpleConnection*> Connections;

	FSimpleConnection* GetFreeConnection();//找闲置Connection
	virtual void SetNonblocking();//设置非阻塞
};