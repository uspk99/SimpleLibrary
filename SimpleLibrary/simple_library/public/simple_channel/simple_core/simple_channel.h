#pragma once
#include "../../simple_array/MyIterator.h"

class FSimpleConnection;
class FSimpleChannel
{
public:
	FSimpleChannel();

	unsigned int GetGuid() { return ID; }
	void SetGuid(unsigned int InGuid);

	void Send(TArray<unsigned char>& InBuffer);
	bool Receive(TArray<unsigned char>& InBuffer);

	void SetConnection(FSimpleConnection* InConnection);
protected:
	FSimpleConnection* Connection;
	unsigned int ID;
};