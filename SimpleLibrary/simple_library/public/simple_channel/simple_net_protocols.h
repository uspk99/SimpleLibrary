#pragma once
#include "simple_protocols_definition.h"

DEFINITION_SIMPLE_PROTOCOLS(Hello,0)		//�ͻ��������������Hello��֤
DEFINITION_SIMPLE_PROTOCOLS(Challenge,1)	//��������֤�ͻ���
DEFINITION_SIMPLE_PROTOCOLS(Login,2)		//�ͻ��˷��͵�¼����
DEFINITION_SIMPLE_PROTOCOLS(Welcome,3)		//�����������¼
DEFINITION_SIMPLE_PROTOCOLS(Join,4)		//�ͻ��˵�¼�ɹ�

DEFINITION_SIMPLE_PROTOCOLS(Debug,5)		//���͵�����Ϣ
DEFINITION_SIMPLE_PROTOCOLS(Failure,6)		//����
DEFINITION_SIMPLE_PROTOCOLS(Upgrade,7)		//�汾��һ��

DEFINITION_SIMPLE_PROTOCOLS(HearBeat,8)	//����
DEFINITION_SIMPLE_PROTOCOLS(Close,9)		//�ͻ��˹ر�����Э��

//����  0 1 2 3 4 888888������ 9