#pragma once
#include "simple_core/simple_channel.h"
#include "simple_core/simple_io_stream.h"
#include "simple_net_type.h"

#define DEFINITION_SIMPLE_BUFFER\
		TArray<unsigned char> Buffer;\
		FSimpleIOStream Stream(Buffer);

template<unsigned int ProtocolsType>
class FSimpleProtocols{};//Э��


class FRecursionMessageInfo
{
public:
	template<typename ...ParamTypes>
	static unsigned int GetBuildParams(ParamTypes &...Params)
	{
		return sizeof ...(Params);//��ȡ��������
	}

	template<typename ...ParamTypes>
	static void BuildSendParams(FSimpleIOStream& InStream, ParamTypes &...Params) {}

	template<class T,typename ...ParamTypes>
	static void BuildSendParams(FSimpleIOStream& InStream, T& FirstParam, ParamTypes &...Params)
	{
		InStream << FirstParam;
		BuildSendParams(InStream, Params...);
	}

	template<typename ...ParamTypes>
	static void BuildReceiveParams(FSimpleIOStream& InStream, ParamTypes &...Params) {}

	template<class T, typename ...ParamTypes>
	static void BuildReceiveParams(FSimpleIOStream& InStream, T& FirstParam, ParamTypes &...Params)
	{
		InStream >> FirstParam;
		BuildReceiveParams(InStream, Params...);
	}

};

#define DEFINITION_SIMPLE_PROTOCOLS(ProtocolsName,ProtocolsNumber)\
enum {SP_##ProtocolsName = ProtocolsNumber};\
template<>\
class FSimpleProtocols<ProtocolsNumber>\
{\
public:\
	template<typename ...ParamTypes>\
	static void Send(FSimpleChannel* InChannel, ParamTypes &...Params)\
	{\
		DEFINITION_SIMPLE_BUFFER;\
		FSimpleBunchHead Head;\
		Head.Protocols = ProtocolsNumber;\
		Head.ParamNum = FRecursionMessageInfo::GetBuildParams(Params...);\
		Head.ChannelID = InChannel->GetGuid();\
		Stream << Head;\
		FRecursionMessageInfo::BuildSendParams(Stream, Params...);\
		InChannel->Send(Buffer);\
	}\
	template<typename ...ParamTypes>\
	static void Receive(FSimpleChannel* InChannel, ParamTypes &...Params)\
	{\
		DEFINITION_SIMPLE_BUFFER;\
		if (InChannel->Receive(Buffer))\
		{\
			Stream.Seek(sizeof(FSimpleBunchHead));\
			FRecursionMessageInfo::BuildReceiveParams(Stream, Params...);\
		}\
	}\
};
//ƫ�ػ�
#if 0
enum
{
	Debug = 0,
};

template<>
class FSimpleProtocols<Debug>
{
public:

	template<typename ...ParamTypes>
	static void Send(FSimpleChannel* InChannel,ParamTypes &...Params)
	{
		//TArray<unsigned char> Buffer;
		//FSimpleIOStream Stream(Buffer);
		DEFINITION_SIMPLE_BUFFER;
		FSimpleBunchHead Head;
		Head.ProtocolsNumber = 0;
		Head.ParamNum = FRecursionMessageInfo::GetBuildParams(Params...);
		Head.ChannelID = InChannel->GetGuid();

		Stream << Head;
		FRecursionMessageInfo::BuildSendParams(Stream, Params...);
		InChannel->Send(Buffer);		
	}

	template<typename ...ParamTypes>
	static void Receive(FSimpleChannel* InChannel, ParamTypes &...Params)
	{
		DEFINITION_SIMPLE_BUFFER;
		if (InChannel->Receive(Buffer))
		{
			Buffer.Seek(sizeof(FSimpleBunchHead));//������ͷ
			FRecursionMessageInfo::BuildReceiveParams(Stream, ...Params);
		}
		
	}
};
#endif

#define SIMPLE_PROTOCOLS_SEND(InProtocols,...) FSimpleProtocols<InProtocols>::Send(Channel,__VA_ARGS__);
#define SIMPLE_PROTOCOLS_RECEIVE(InProtocols,...) FSimpleProtocols<InProtocols>::Receive(Channel,__VA_ARGS__);
//���Ͷ�
//int a=1;int b=2;
//std::string data = "xxxxx";
//FSimpleProtocols<Debug>::Send(Channel, a, b, data);
//SIMPLE_PROTOCOLS_SEND(Debug,Channel,a,b,data);
//���ն�
//int a; int b;
//std::string data;
//FSimpleProtocols<Debug>::Receive(Channel, a, b, data);
//if (a == 1) {...}
//����
//DEFINITION_SIMPLE_PROTOCOLS(Debug, 1);
