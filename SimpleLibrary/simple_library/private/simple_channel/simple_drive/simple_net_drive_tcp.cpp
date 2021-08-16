#include "simple_net_drive_tcp.h"
#include "public/simple_c_log/simple_c_log.h"
#include "../simple_net_connection/simple_connection _tcp.h"
#include "public/simple_channel/simple_protocols_definition.h"
#include "public/simple_channel/simple_net_protocols.h"

HANDLE FSimpleTCPNetDrive::CompletionPortHandle = nullptr;

FSimpleTCPNetDrive::FSimpleTCPNetDrive(ESimpleDriveType InDriveType)
	:DriveType(InDriveType)
{
	MainConnection = new FSimpleTCPConnection();
	MainConnection->SetConnectionState(ESimpleConnectionState::Free);
}

void FSimpleTCPNetDrive::SetNonblocking()
{
	//设置非阻塞
	{
		//设置主Connection非阻塞模式
		//0为阻塞 1为非阻塞
		unsigned long UL = 1;
		int Ret = ioctlsocket(MainConnection->GetSocket(), FIONBIO, &UL);
		if (Ret == SOCKET_ERROR)
		{
			log_error("%s", "SetNonblocking(设置非阻塞)失败~~\n");
		}
	}
}

void HandShake(FSimpleConnection* InLink)
{
	if (!InLink)
	{
		return;
	}
	//Head改为非指针,防止右边的变量被清掉后指针无效,
	//比如下面没有额外参数的情况，头部也会一起memset清掉
	// 
	//int InLen = *((int*)InLink->GetConnectionIoData().Buffer);
	//auto TestPtr = InLink->GetConnectionIoData().Buffer;
	//auto TestPtr1 = InLink->GetConnectionIoData().Buffer+4;
	//InLen = ntohl(InLen);

	FSimpleBunchHead Head = *((FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer);
	//if (FSimpleBunchHead* Head = (FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer)

	DWORD ll = InLink->GetConnectionIoData().Len;

	if (Head.ParamNum == 0)
		//协议里没有额外内容 清除buffer区域
	{
		memset(InLink->GetConnectionIoData().Buffer, 0, 1024);
		InLink->GetConnectionIoData().Len = 0;
	}
	//获取主通道
	if (FSimpleChannel* Channel = InLink->GetMainChannnel())
	{
		if (InLink->GetDriveType() == ESimpleDriveType::DRIVETYPE_LISTEN)
		{//服务端
			switch (Head.Protocols)
			{
				case SP_Hello:
				{
					std::string VersionRemote;
					SIMPLE_PROTOCOLS_RECEIVE(SP_Hello, VersionRemote);
					if (VersionRemote == "1.0.1")
					{
						SIMPLE_PROTOCOLS_SEND(SP_Challenge);
						log_log("Server: [Challenge] : % s", InLink->GetAddString().c_str());
					}
					break;
				}
				case SP_Login:
				{
					std::vector<int> Channels;
					SIMPLE_PROTOCOLS_RECEIVE(SP_Login, Channels);
					if (Channels.size()==10)
					{
						InLink->SetConnectionState(ESimpleConnectionState::Login);
						auto ChannelLists = InLink->GetChannels();
						int i = 0;
						for (auto &Tmp:*ChannelLists)
						{
							Tmp.SetGuid(i);
							i++;
						}
						SIMPLE_PROTOCOLS_SEND(SP_Welcome);
						log_log("Server: [Welcome] : % s", InLink->GetAddString().c_str());
						break;
					}
				}
				case SP_Join:
				{
					InLink->SetConnectionState(ESimpleConnectionState::Join);
					log_success("Server: [Join] : % s", InLink->GetAddString().c_str());
					break;
				}
			}
		}
		else//客户端
		{
			switch (Head.Protocols)
			{
				case SP_Challenge:
				{
					std::vector<int> Channels;
					InLink->GetChannnelActiveID(Channels);
					SIMPLE_PROTOCOLS_SEND(SP_Login, Channels);
					InLink->SetConnectionState(ESimpleConnectionState::Login);
					log_log("Client:[Login] :%s", InLink->GetAddString().c_str());
					break;
				}
				case SP_Welcome:
				{
					InLink->SetConnectionState(ESimpleConnectionState::Join);
					SIMPLE_PROTOCOLS_SEND(SP_Join);
					log_log("Client: [Join] : % s", InLink->GetAddString().c_str());
					//设置心跳
					break;
				}
				default:
					break;
			}
		}
	}
}

unsigned int __stdcall Run(void* content)
{
	for (;;)
	{
		DWORD IOSize = -1;//一次IO传递的字节数
		LPOVERLAPPED lpOverlapped = NULL;
		FSimpleTCPConnection* InLink = NULL;

		bool Ret = GetQueuedCompletionStatus(//获取队列状态
			FSimpleTCPNetDrive::CompletionPortHandle,
			&IOSize,
			(LPDWORD)&InLink,
			&lpOverlapped,
			INFINITE);//INFINITE 无限等待
		
		FSimpleBunchHead Head = *((FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer);
		std::cout << "IOSize：" << IOSize << "  Headlength: " << Head.ParamLength << std::endl;

		if (InLink == NULL && lpOverlapped == NULL)
		{
			break;
		}
		if (Ret)
		{
			if (IOSize == 0)
			{
				continue;
			}
			FSimpleIoData* pData = CONTAINING_RECORD(lpOverlapped, FSimpleIoData, Overlapped);
			//当得到了指向OVERLAPPED结构的指针以后，可以用CONTAINING RECORD宏取出其中指向扩展结构的指针。
			switch (pData->Type)
			{
			case 0://接收信息
			{
				InLink->GetConnectionIoData().Len = IOSize;
				InLink->GetConnectionIoData().Buffer[IOSize] = '\0';//最长位置以\0结尾

				if (InLink->GetConnectionState()==ESimpleConnectionState::Join)
					//验证客户端是否已经join
				{
					//业务逻辑
				}
				else
				{				
					HandShake(InLink);
				}
				//char buffer[1024] = { 0 };
				break;
			}
			//发送信息
#if 0
			case 1:
			{
				printf(InLink->GetConnectionIoData().Buffer);
				InLink->GetConnectionIoData().Len = 0;

				if (!InLink->Recv())
				{
					InLink->Send();
					//ListRemove(InLink);
				}
				break;
			}
#endif
			}
		}
		else
		{
			DWORD Msg = GetLastError();
			if (Msg == WAIT_TIMEOUT)//是否等待超时
			{
				continue;
			}
			else if (lpOverlapped != NULL)//产生重叠
			{
				//ListRemove(InLink);
			}
			else
			{
				break;
			}
		}
	}

	return 0;
}

bool FSimpleTCPNetDrive::Init()
{
	if (DriveType==ESimpleDriveType::DRIVETYPE_LISTEN)
	{

		//完成端口
		if ((CompletionPortHandle = CreateIoCompletionPort(//创建IO完成端口
			INVALID_HANDLE_VALUE,	//打开的文件句柄
			NULL,//现有完成端口
			0,//
			0)) == NULL)			//允许最大线程数
		{
			//GetLastError();

			log_error("%s", "创建完成端口失败~~\n");
			return false;
		}
		//创建线程数目

		for (int i = 0; i < (2 * 2); i++)
		{
			hThreadHandle[i] = (HANDLE)_beginthreadex(
				NULL,//安全属性，null表示安全性
				0, //线程堆栈大小，0为默认
				Run, //启动函数
				CompletionPortHandle,//线程参数（结构，指针）
				0, //当前线程执行状态 0立即执行 CREATE_SUSPENDED表示创建后挂起
				NULL);//接收线程ID，成功为ID，失败为0
		}
	}

	//更新库
	int Ret = 0;
	if ((Ret = WSAStartup(MAKEWORD(2, 1),
		&WsaData)) != 0)//版本 2.1	
	{
		log_error("%s", "初始化库失败~~\n");
		return false;
	}
	//执行connection初始化
	MainConnection->Init();
	MainConnection->SetDriveType(DriveType);

	if (DriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//创建SOCKET
		MainConnection->GetSocket() = INVALID_SOCKET;
		if ((MainConnection->GetSocket() = WSASocket(
			AF_INET,//地址簇的描述
			SOCK_STREAM,//当前的套接字 TCP  SOCK_DGRAM UDP
			0,//指定协议 0为不使用
			NULL,//套接字特性 NULL 默认
			0,//套接字标识符，保留给未使用的套接字组
			WSA_FLAG_OVERLAPPED//套接字描述 重叠IO
		)) == INVALID_SOCKET)
		{
			log_error("%s", "创建监听SOCKET失败~~\n");
			WSACleanup();
			return false;
		}
		//注册地址
		MainConnection->GetConnectionAddr().sin_family = AF_INET;//ipv4协议族
		MainConnection->GetConnectionAddr().sin_addr.S_un.S_addr = htonl(INADDR_ANY);//0.0.0.0所有地址
		MainConnection->GetConnectionAddr().sin_port = htons(98591);//端口

			//绑定
		if (bind(MainConnection->GetSocket(),
			(SOCKADDR*)&MainConnection->GetConnectionAddr(),
			sizeof(MainConnection->GetConnectionAddr())) == SOCKET_ERROR)
			//判断绑定是否成功
		{
			log_error("%s", "绑定主连接失败~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}

		//监听
		if (listen(MainConnection->GetSocket(), SOMAXCONN))//监听 客户端数量最大
		{
			log_error("%s", "启动监听失败~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}

		//初始化通道
		for (int i=0;i<2000;i++)
		{
			Connections.insert(std::make_pair(i,new FSimpleTCPConnection()));
			Connections[i]->Init();
			Connections[i]->SetDriveType(DriveType);
		}
	}
	else
	{
		MainConnection->GetSocket() = socket(
			AF_INET,//地址簇internetwork: UDP, TCP, etc.
			SOCK_STREAM,//TCP
			IPPROTO_TCP);//TCP协议簇		
		if (!MainConnection->GetSocket() == INVALID_SOCKET)
		{
			log_error("%s", "创建客户端SOCKET失败~~\n");
			WSACleanup();
			return false;
		}
		MainConnection->GetConnectionAddr().sin_family = AF_INET;//ipv4协议族
		//客户端设置服务端地址 默认地址，（公网IP)
		MainConnection->GetConnectionAddr().sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		//端口
		MainConnection->GetConnectionAddr().sin_port = htons(98591);//保证端口一致

		//连接
		if (connect(MainConnection->GetSocket(),
			(SOCKADDR*)&MainConnection->GetConnectionAddr(),
			sizeof(MainConnection->GetConnectionAddr())) == SOCKET_ERROR)
		{
			log_error("%s", "客户端连接失败~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}
		//向服务器发送验证
#if 0
		//char ClientBuffer[1024] = { 0 };
		//sprintf_s(ClientBuffer, "Hello I am InLink %d",MainConnection->GetSocket());
		//send(MainConnection->GetSocket(),ClientBuffer,sizeof(ClientBuffer),0);
#endif		
		if (FSimpleChannel* Channel = MainConnection->GetMainChannnel())
		{
			std::string v = "1.0.1";
			SIMPLE_PROTOCOLS_SEND(SP_Hello, v);

			log_log("Client send [Hello] to Server[addr: %s]~~ \n", MainConnection->GetAddString().c_str());
		}
	}
	//设置非阻塞
	SetNonblocking();
	return true;
}

void FSimpleTCPNetDrive::Tick(double InTimeInterval)
{
	Super::Tick(InTimeInterval);

	if (DriveType==ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//做接收检测
#if 1
		for (auto &Tmp : Connections)
		{
			if (Tmp.second->GetConnectionState()!=ESimpleConnectionState::Free)
			{
				Tmp.second->Recv();
			}
		}
#endif


		//iocp投递
		SOCKET ClientAccept = INVALID_SOCKET;
		SOCKADDR_IN ClientAddr;
		int ClientAddrLen = sizeof(ClientAddr);

		if ((ClientAccept=WSAAccept(
			MainConnection->GetSocket(),
			(SOCKADDR*)&ClientAddr,
			&ClientAddrLen,
			NULL,
			0))==SOCKET_ERROR)
		{
			//log_error("%s", "接收客户端投递失败~~\n");
			return;
		}
		//查找闲置connection
		if (FSimpleConnection* FreeConnection = GetFreeConnection())
		{
			//绑定完成端口
			if (CreateIoCompletionPort(
				(HANDLE)ClientAccept,//打开的文件句柄
				CompletionPortHandle,//现有完成端口
				(DWORD)FreeConnection,	//每个句柄用户定义的完成密钥，包含在指定文件句柄的每个I/O完成数据包中
				0) == NULL)//允许最大线程数
			{
				log_error("%s", "绑定端口失败~~\n");
				return;
			}
			//绑定Socket
			FreeConnection->GetSocket() = ClientAccept;
			FreeConnection->GetConnectionAddr() = ClientAddr;

			//接收
			if (!FreeConnection->Recv())
			{
				//ListRemove(InClient);
				log_error("%s", "接收客户端失败~~\n");
				return;
			}
			else
			{
				FreeConnection->SetConnectionState(ESimpleConnectionState::Version_Verfication);
				//FreeConnection->SetConnectionState(ESimpleConnectionState::Join);
				//log_success("服务端接受客户端成功%s\n", MainConnection->GetConnectionIoData().Buffer);
			}
		}
	}
	else
	{
		if (MainConnection->Recv())
		{
			//做解析
			
			if (MainConnection->GetConnectionState()!=ESimpleConnectionState::Join)
			{
				//log_success("客户端链接成功%s\n",MainConnection->GetConnectionIoData().Buffer);
				HandShake(MainConnection);
			}
			else
			{
				//做上层业务逻辑
			}
		}
	}
}
