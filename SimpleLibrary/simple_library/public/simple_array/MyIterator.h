#pragma once
#include <corecrt_malloc.h>

template<class ContainerType, typename ElementType>
class TIndexedContainerIterator
{
public:
	typedef TIndexedContainerIterator<ContainerType, ElementType> TIterator;

	//构造
	TIndexedContainerIterator(ContainerType& InContainer, int InIndex = 0)
		:Container(InContainer), Index(InIndex)
	{}
	//深拷贝
	TIndexedContainerIterator(const TIterator& InIterator)
		:Container(InIterator.Container)
		, Index(InIterator.Index)
	{}
	//It!=XXX.end()
	bool operator!=(const TIterator& InIterator)
	{
		return Container[Index] != InIterator.Container[InIterator.Index];
	}
	//++ --
	TIterator& operator++(int)
	{
		++Index;
		return *this;
	}
	TIterator& operator--(int)
	{
		--Index;
		return *this;
	}
	//auto dd=TArray<int>::Iterator
	//赋值
	TIterator& operator=(const TIterator& InIterator)
	{
		Index = InIterator.Index;
		Container = InIterator.Container;
		return *this;
	}
	//访问
	ElementType& operator*()
	{
		return *Container[Index];
	}
protected:
	ContainerType& Container;
	int Index;
};

template<typename ElementType>
class TArray
{
public:
	typedef TIndexedContainerIterator<TArray<ElementType>, ElementType> TIterator;
	
	TArray()
		:Data(nullptr)
		,Size(0)
	{
		//Data = (ElementType**)malloc(sizeof(int) * Allocation);//分配下标
		//memset(Data, 0, sizeof(int) * Allocation);
		//for (int i=0;i<Allocation;i++)
		//{
		//	Data[i] = (ElementType*)malloc(sizeof(ElementType));
		//}
	}
	~TArray()
	{
		free(Data);
	}
	TIterator Begin()
	{
		return TIterator(*this, 0);
	}

	TIterator End()
	{
		return TIterator(*this, Size);
	}

	int Num()
	{
		return Size;
	}

	int AddUninitialized(int InLength)
		//分配空间
	{
		if (Size == 0)
		{
			Data = (ElementType*)malloc(InLength);
		}
		else
		{
			Data = (ElementType*)realloc(Data,InLength + Size);
		}
		//现在不是二维数组
		//memset(Data[Size], 0, sizeof(ElementType));
		memset(&Data[Size], 0, sizeof(ElementType));
		int LastPos = Size;
		Size += InLength;
		return LastPos;
	}

	//ArrayA.Add(FTestA(1));可用
	void Add(ElementType&& InType)
	{
		Add(InType);
	}
	void Add(ElementType& InType)
	{
		if (Size==0)
		{
			Data = (ElementType*)malloc(sizeof(ElementType));
		}
		//判断内存够不够，是否需要分配
		else
		{
			Data = (ElementType**)realloc(Data, sizeof(ElementType)*(Size+1));
		}
		memset(Data[Size], 0, sizeof(ElementType));
		memcpy(Data[Size], &InType, sizeof(ElementType));
		Size++;
	}
	//void RemoveAt(int Index)
	//{
	//	memset(Data[Size], 0, sizeof(ElementType));//清零
	//	Size--;
	//}

	ElementType &operator[](int Index)
	{
		return Data[Index];
	}

	ElementType* GetData()//返回首地址
	{
		return Data;
	}
	
protected:
	ElementType* Data;//连续内存
	int Size;
};