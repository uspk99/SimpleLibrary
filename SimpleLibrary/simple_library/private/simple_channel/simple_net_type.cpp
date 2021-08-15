#pragma once
#include "../../public/simple_channel/simple_net_type.h"

FSimpleIoData::FSimpleIoData()
{

	ZeroMemory(this, sizeof(FSimpleIoData));//对自己初始化

}

FSimpleBunchHead::FSimpleBunchHead()
	:Protocols(0),
	ChannelID(0),
	ParamNum(0)
{

}
