


#pragma once
#include "../../simple_core_minimal/simple_c_core/simple_core_minimal.h"
#include "../simple_net_type.h"
#include "../../simple_array/MyIterator.h"
#include "../../simple_cpp_core_minimal/simple_cpp_core_minimal.h"
#include "public/simple_channel/simple_core/simple_channel.h"

class FSimpleConnection
{
public:
	FSimpleConnection();
	virtual bool Init();

	virtual BOOL Recv();
	virtual BOOL Send();

	void SetBuffer(TArray<unsigned char>& InBuffer);
	void RecvBuffer(TArray<unsigned char>& InBuffer);

	void SetConnectionState(const ESimpleConnectionState& InNewConnectionState);

	FSimpleChannel* GetMainChannnel();
	FSimpleChannel* GetChannnel(int InID);

	void SetDriveType(const ESimpleDriveType InDriveType);
	void GetChannnelActiveID(std::vector<int>& InIDs);
	std::string GetAddString();

	std::list<FSimpleChannel>* GetChannels();//ªÒ»°channels¡¥±Ì

	FORCEINLINE SOCKET &GetSocket() { return Socket; }
	FORCEINLINE SOCKADDR_IN &GetConnectionAddr() { return ConnectAddr; }
	FORCEINLINE ESimpleConnectionState& GetConnectionState() { return ConnectionState; }
	FORCEINLINE FSimpleIoData& GetConnectionIoData() { return IOData; }
	FORCEINLINE ESimpleDriveType& GetDriveType() { return DriverType; }
protected:
	
	SOCKADDR_IN ConnectAddr;
	ESimpleConnectionState ConnectionState;
	FSimpleIoData IOData;
	SOCKET Socket;
	std::list<FSimpleChannel> Channels;

	ESimpleDriveType DriverType;

	int DataLength;
};