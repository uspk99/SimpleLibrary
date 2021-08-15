#pragma once
#include <map>
#include "../simple_core_minimal/simple_c_guid/simple_guid.h"
//函数代理接口
template<class TReturn, typename ...ParamTypes>
class FDelegateBase
{
public:
	virtual TReturn Execute(ParamTypes &&...Params)
	{
		return TReturn();
	}
};

//函数代理
template<class TReturn, typename ...ParamTypes>
class FFunctionDelegate:public FDelegateBase<TReturn, ParamTypes...>
{
public:
	FFunctionDelegate(TReturn(*InFuncation)(ParamTypes...))
		:Funcation(InFuncation)
	{}
	virtual TReturn Execute(ParamTypes &&...Params)
	{
		return(*Funcation)(std::forward<ParamTypes>(Params)...);
	}

private:

	TReturn(* Funcation)(ParamTypes...);
};



//最简单的支持多参数代理
//对象代理
template<class TObjectType, class TReturn, typename ...ParamTypes>
class FObjectDelegate :public FDelegateBase<TReturn, ParamTypes...>
{
public:
	FObjectDelegate(TObjectType* InObject, TReturn(TObjectType::* InFuncation)(ParamTypes...))
		:Object(InObject), Funcation(InFuncation)
	{}
	virtual TReturn Execute(ParamTypes &&...Params)
	{
		return(Object->*Funcation)(std::forward<ParamTypes>(Params)...);
	}

private:
	TObjectType* Object;
	TReturn(TObjectType::* Funcation)(ParamTypes...);
};

//工厂
template<class TReturn, typename ...ParamTypes>
class FDelegate
{
public:
	FDelegate():CurrentDelegatePtr(nullptr)
	{

	}
	~FDelegate()//临时对象销毁后如果清空代理，Execute会崩溃
	{
		//CheckDelegate();
	}
	//释放代理
	void ReleaseDelegate()
	{
		if (CurrentDelegatePtr)
		{
			delete CurrentDelegatePtr;
			CurrentDelegatePtr = nullptr;
		}
	}
	//执行
	virtual TReturn Execute(ParamTypes &&...Params)
	{
		return CurrentDelegatePtr->Execute(std::forward<ParamTypes>(Params)...);
		//深拷贝问题，出现崩溃
	}
	FDelegate<TReturn, ParamTypes...>& operator=(FDelegate<TReturn, ParamTypes...>& InDelegate)
	{
		CurrentDelegatePtr = InDelegate.CurrentDelegatePtr;
		return *this;
	}

	//判断是否有意义
	bool IsBound()
	{
		return CurrentDelegatePtr != nullptr;
	}
	//创建
	template<class TObjectType>
	static FDelegate<TReturn, ParamTypes...> Create(TObjectType* InObject, TReturn(TObjectType::* InFuncation)(ParamTypes...))
	{
		FDelegate<TReturn, ParamTypes...> DelegateInstance;
		DelegateInstance.Bind(InObject, InFuncation);
		return DelegateInstance;
	}

	static FDelegate<TReturn, ParamTypes...> Create(TReturn(*InFuncation)(ParamTypes...))
	{
		FDelegate<TReturn, ParamTypes...> DelegateInstance;
		DelegateInstance.Bind(InFuncation);
		return DelegateInstance;
	}

	//绑定
	template<class TObjectType>
	void Bind(TObjectType* InObject, TReturn(TObjectType::* InFuncation)(ParamTypes...))
	{
		ReleaseDelegate();
		CurrentDelegatePtr = new FObjectDelegate<TObjectType, TReturn, ParamTypes...>(InObject, InFuncation);
	}

	void Bind(TReturn(*InFuncation)(ParamTypes...))
	{
		ReleaseDelegate();
		CurrentDelegatePtr = new FFunctionDelegate<TReturn, ParamTypes...>(InFuncation);
	}

private:
	FDelegateBase<TReturn, ParamTypes...>* CurrentDelegatePtr;
};

//template<class TObjectType, class TReturn, typename ...ParamTypes>
//FObjectDelegate<TObjectType, TReturn, ParamTypes...> CreateDelegate(TObjectType* InObject, TReturn(TObjectType::* InFuncation)(ParamTypes...))
//{
//	return FObjectDelegate<TObjectType, TReturn, ParamTypes...>(InObject, InFuncation);//指针拷贝
//}

//单播代理
template<class TReturn, typename ...ParamTypes>
class FSingleDelegate :public FDelegate<TReturn, ParamTypes...>
{
public:
	FSingleDelegate()
		:FDelegate<TReturn, ParamTypes...>()
	{}
};

//代理handle，
struct FDelegateHandle
{
	FDelegateHandle()
	{
		create_guid(&Guid);
	}
	//重载
	friend bool operator<(const FDelegateHandle& K1, const FDelegateHandle& K2)
	{
		return K1.Guid.a < K2.Guid.a;
	}

	simple_c_guid Guid;
};



//多播代理
template<class TReturn, typename ...ParamTypes>
class FMulticastDelegate :public std::map<FDelegateHandle,FDelegate<TReturn, ParamTypes...>>
{
	typedef FDelegate<TReturn, ParamTypes...> TDelegate;
public:
	FMulticastDelegate()
	{}

	//移除
	void RemoveDelegate(const FDelegateHandle& Handle)
	{
		this->erase(Handle);
	}

	//添加函数
	template<class TObjectType>
	const FDelegateHandle &AddFunction(TObjectType* InObject, TReturn(TObjectType::* InFuncation)(ParamTypes...))
	{
		//生成guid
		FDelegateHandle Handle;;

		this->insert(std::make_pair(Handle, TDelegate()));//添加

		TDelegate &InDelegate = this->at(Handle);
		InDelegate.Bind(InObject, InFuncation);

		return Handle;
	}

	const FDelegateHandle &AddFunction(TReturn(*InFuncation)(ParamTypes...))
	{
		//生成guid
		FDelegateHandle Handle;;

		this->insert(std::make_pair(Handle,TDelegate()));//添加

		TDelegate& InDelegate = this->at(Handle);
		InDelegate.Bind(InFuncation);

		return Handle;
	}

	//执行，广播
	void Broadcast(ParamTypes &&...Params)
	{
		for (auto& Tmp : *this)
		{
			Tmp.second.Execute(std::forward<ParamTypes>(Params)...);
		}
	}

	//释放
	void ReleaseDelegate()
	{
		for (auto& Tmp : *this)
		{
			Tmp.second.ReleaseDelegate();
		}
	}
	//移除代理

};

//宏定义 生成对象
#define Simple_Single_DELEGATE(Name,Return,...) FSingleDelegate<Return,__VA_ARGS__> Name;
//宏定义 定义类
#define DEFINITION_SIMPLE_SINGLE_DELEGATE(DefinitionName,Return,...) class DefinitionName :public FSingleDelegate<Return,__VA_ARGS__> {};
//宏定义 定义类
#define DEFINITION_MULTICAST_SINGLE_DELEGATE(DefinitionName,Return,...) class DefinitionName :public FMulticastDelegate<Return,__VA_ARGS__> {};