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
	//���÷�����
	{
		//������Connection������ģʽ
		//0Ϊ���� 1Ϊ������
		unsigned long UL = 1;
		int Ret = ioctlsocket(MainConnection->GetSocket(), FIONBIO, &UL);
		if (Ret == SOCKET_ERROR)
		{
			log_error("%s", "SetNonblocking(���÷�����)ʧ��~~\n");
		}
	}
}

void HandShake(FSimpleConnection* InLink)
{
	if (!InLink)
	{
		return;
	}
	//Head��Ϊ��ָ��,��ֹ�ұߵı����������ָ����Ч,
	//��������û�ж�������������ͷ��Ҳ��һ��memset���
	// 
	//int InLen = *((int*)InLink->GetConnectionIoData().Buffer);
	//auto TestPtr = InLink->GetConnectionIoData().Buffer;
	//auto TestPtr1 = InLink->GetConnectionIoData().Buffer+4;
	//InLen = ntohl(InLen);

	FSimpleBunchHead Head = *((FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer);
	//if (FSimpleBunchHead* Head = (FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer)

	DWORD ll = InLink->GetConnectionIoData().Len;

	if (Head.ParamNum == 0)
		//Э����û�ж������� ���buffer����
	{
		memset(InLink->GetConnectionIoData().Buffer, 0, 1024);
		InLink->GetConnectionIoData().Len = 0;
	}
	//��ȡ��ͨ��
	if (FSimpleChannel* Channel = InLink->GetMainChannnel())
	{
		if (InLink->GetDriveType() == ESimpleDriveType::DRIVETYPE_LISTEN)
		{//�����
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
		else//�ͻ���
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
					//��������
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
		DWORD IOSize = -1;//һ��IO���ݵ��ֽ���
		LPOVERLAPPED lpOverlapped = NULL;
		FSimpleTCPConnection* InLink = NULL;

		bool Ret = GetQueuedCompletionStatus(//��ȡ����״̬
			FSimpleTCPNetDrive::CompletionPortHandle,
			&IOSize,
			(LPDWORD)&InLink,
			&lpOverlapped,
			INFINITE);//INFINITE ���޵ȴ�
		
		FSimpleBunchHead Head = *((FSimpleBunchHead*)InLink->GetConnectionIoData().Buffer);
		std::cout << "IOSize��" << IOSize << "  Headlength: " << Head.ParamLength << std::endl;

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
			//���õ���ָ��OVERLAPPED�ṹ��ָ���Ժ󣬿�����CONTAINING RECORD��ȡ������ָ����չ�ṹ��ָ�롣
			switch (pData->Type)
			{
			case 0://������Ϣ
			{
				InLink->GetConnectionIoData().Len = IOSize;
				InLink->GetConnectionIoData().Buffer[IOSize] = '\0';//�λ����\0��β

				if (InLink->GetConnectionState()==ESimpleConnectionState::Join)
					//��֤�ͻ����Ƿ��Ѿ�join
				{
					//ҵ���߼�
				}
				else
				{				
					HandShake(InLink);
				}
				//char buffer[1024] = { 0 };
				break;
			}
			//������Ϣ
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
			if (Msg == WAIT_TIMEOUT)//�Ƿ�ȴ���ʱ
			{
				continue;
			}
			else if (lpOverlapped != NULL)//�����ص�
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

		//��ɶ˿�
		if ((CompletionPortHandle = CreateIoCompletionPort(//����IO��ɶ˿�
			INVALID_HANDLE_VALUE,	//�򿪵��ļ����
			NULL,//������ɶ˿�
			0,//
			0)) == NULL)			//��������߳���
		{
			//GetLastError();

			log_error("%s", "������ɶ˿�ʧ��~~\n");
			return false;
		}
		//�����߳���Ŀ

		for (int i = 0; i < (2 * 2); i++)
		{
			hThreadHandle[i] = (HANDLE)_beginthreadex(
				NULL,//��ȫ���ԣ�null��ʾ��ȫ��
				0, //�̶߳�ջ��С��0ΪĬ��
				Run, //��������
				CompletionPortHandle,//�̲߳������ṹ��ָ�룩
				0, //��ǰ�߳�ִ��״̬ 0����ִ�� CREATE_SUSPENDED��ʾ���������
				NULL);//�����߳�ID���ɹ�ΪID��ʧ��Ϊ0
		}
	}

	//���¿�
	int Ret = 0;
	if ((Ret = WSAStartup(MAKEWORD(2, 1),
		&WsaData)) != 0)//�汾 2.1	
	{
		log_error("%s", "��ʼ����ʧ��~~\n");
		return false;
	}
	//ִ��connection��ʼ��
	MainConnection->Init();
	MainConnection->SetDriveType(DriveType);

	if (DriveType == ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//����SOCKET
		MainConnection->GetSocket() = INVALID_SOCKET;
		if ((MainConnection->GetSocket() = WSASocket(
			AF_INET,//��ַ�ص�����
			SOCK_STREAM,//��ǰ���׽��� TCP  SOCK_DGRAM UDP
			0,//ָ��Э�� 0Ϊ��ʹ��
			NULL,//�׽������� NULL Ĭ��
			0,//�׽��ֱ�ʶ����������δʹ�õ��׽�����
			WSA_FLAG_OVERLAPPED//�׽������� �ص�IO
		)) == INVALID_SOCKET)
		{
			log_error("%s", "��������SOCKETʧ��~~\n");
			WSACleanup();
			return false;
		}
		//ע���ַ
		MainConnection->GetConnectionAddr().sin_family = AF_INET;//ipv4Э����
		MainConnection->GetConnectionAddr().sin_addr.S_un.S_addr = htonl(INADDR_ANY);//0.0.0.0���е�ַ
		MainConnection->GetConnectionAddr().sin_port = htons(98591);//�˿�

			//��
		if (bind(MainConnection->GetSocket(),
			(SOCKADDR*)&MainConnection->GetConnectionAddr(),
			sizeof(MainConnection->GetConnectionAddr())) == SOCKET_ERROR)
			//�жϰ��Ƿ�ɹ�
		{
			log_error("%s", "��������ʧ��~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}

		//����
		if (listen(MainConnection->GetSocket(), SOMAXCONN))//���� �ͻ����������
		{
			log_error("%s", "��������ʧ��~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}

		//��ʼ��ͨ��
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
			AF_INET,//��ַ��internetwork: UDP, TCP, etc.
			SOCK_STREAM,//TCP
			IPPROTO_TCP);//TCPЭ���		
		if (!MainConnection->GetSocket() == INVALID_SOCKET)
		{
			log_error("%s", "�����ͻ���SOCKETʧ��~~\n");
			WSACleanup();
			return false;
		}
		MainConnection->GetConnectionAddr().sin_family = AF_INET;//ipv4Э����
		//�ͻ������÷���˵�ַ Ĭ�ϵ�ַ��������IP)
		MainConnection->GetConnectionAddr().sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		//�˿�
		MainConnection->GetConnectionAddr().sin_port = htons(98591);//��֤�˿�һ��

		//����
		if (connect(MainConnection->GetSocket(),
			(SOCKADDR*)&MainConnection->GetConnectionAddr(),
			sizeof(MainConnection->GetConnectionAddr())) == SOCKET_ERROR)
		{
			log_error("%s", "�ͻ�������ʧ��~~\n");
			closesocket(MainConnection->GetSocket());
			WSACleanup();
			return false;
		}
		//�������������֤
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
	//���÷�����
	SetNonblocking();
	return true;
}

void FSimpleTCPNetDrive::Tick(double InTimeInterval)
{
	Super::Tick(InTimeInterval);

	if (DriveType==ESimpleDriveType::DRIVETYPE_LISTEN)
	{
		//�����ռ��
#if 1
		for (auto &Tmp : Connections)
		{
			if (Tmp.second->GetConnectionState()!=ESimpleConnectionState::Free)
			{
				Tmp.second->Recv();
			}
		}
#endif


		//iocpͶ��
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
			//log_error("%s", "���տͻ���Ͷ��ʧ��~~\n");
			return;
		}
		//��������connection
		if (FSimpleConnection* FreeConnection = GetFreeConnection())
		{
			//����ɶ˿�
			if (CreateIoCompletionPort(
				(HANDLE)ClientAccept,//�򿪵��ļ����
				CompletionPortHandle,//������ɶ˿�
				(DWORD)FreeConnection,	//ÿ������û�����������Կ��������ָ���ļ������ÿ��I/O������ݰ���
				0) == NULL)//��������߳���
			{
				log_error("%s", "�󶨶˿�ʧ��~~\n");
				return;
			}
			//��Socket
			FreeConnection->GetSocket() = ClientAccept;
			FreeConnection->GetConnectionAddr() = ClientAddr;

			//����
			if (!FreeConnection->Recv())
			{
				//ListRemove(InClient);
				log_error("%s", "���տͻ���ʧ��~~\n");
				return;
			}
			else
			{
				FreeConnection->SetConnectionState(ESimpleConnectionState::Version_Verfication);
				//FreeConnection->SetConnectionState(ESimpleConnectionState::Join);
				//log_success("����˽��ܿͻ��˳ɹ�%s\n", MainConnection->GetConnectionIoData().Buffer);
			}
		}
	}
	else
	{
		if (MainConnection->Recv())
		{
			//������
			
			if (MainConnection->GetConnectionState()!=ESimpleConnectionState::Join)
			{
				//log_success("�ͻ������ӳɹ�%s\n",MainConnection->GetConnectionIoData().Buffer);
				HandShake(MainConnection);
			}
			else
			{
				//���ϲ�ҵ���߼�
			}
		}
	}
}
