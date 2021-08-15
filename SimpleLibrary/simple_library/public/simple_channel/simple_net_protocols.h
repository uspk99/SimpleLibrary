#pragma once
#include "simple_protocols_definition.h"

DEFINITION_SIMPLE_PROTOCOLS(Hello,0)		//客户端向服务器发送Hello验证
DEFINITION_SIMPLE_PROTOCOLS(Challenge,1)	//服务器验证客户端
DEFINITION_SIMPLE_PROTOCOLS(Login,2)		//客户端发送登录请求
DEFINITION_SIMPLE_PROTOCOLS(Welcome,3)		//服务器允许登录
DEFINITION_SIMPLE_PROTOCOLS(Join,4)		//客户端登录成功

DEFINITION_SIMPLE_PROTOCOLS(Debug,5)		//发送调试信息
DEFINITION_SIMPLE_PROTOCOLS(Failure,6)		//错误
DEFINITION_SIMPLE_PROTOCOLS(Upgrade,7)		//版本不一致

DEFINITION_SIMPLE_PROTOCOLS(HearBeat,8)	//心跳
DEFINITION_SIMPLE_PROTOCOLS(Close,9)		//客户端关闭链接协议

//流程  0 1 2 3 4 888888。。。 9