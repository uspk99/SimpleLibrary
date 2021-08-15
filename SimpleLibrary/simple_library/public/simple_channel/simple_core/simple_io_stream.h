#pragma once
#include "../../simple_cpp_core_minimal/simple_cpp_core_minimal.h"
#include "../../simple_array/MyIterator.h"

class FSimpleIOStream
{
public:
	FSimpleIOStream(TArray<unsigned char>& InBuffer);
	
	void Wirte(const void* InData, int Inlength);

	template<class T>
	void Read(T& InValue)
	{
		Seek(0);
		InValue = *(T*)Ptr;
		Seek(sizeof(T));
	}

	void Seek(int InPos);//找起点

	TArray<unsigned char>& Buffer;

	unsigned char* Begin();
	unsigned char* End();
	unsigned char* Tall();//当前位置
private:
	unsigned char* Ptr;//指向里面的字节

public:


	template<class T>
	FSimpleIOStream& operator<<(const T& InValue)
	{
		Wirte(&InValue, sizeof(T));
		return *this;
	}

	template<class T>
	FSimpleIOStream& operator>>(T& InValue)
	{
		Read<T>(InValue);
		return *this;
	}

	FSimpleIOStream& operator<<(const std::string& InValue);
	FSimpleIOStream& operator<<(const std::wstring& InValue);

	FSimpleIOStream& operator>>(std::string& InValue);
	FSimpleIOStream& operator>>(std::wstring& InValue);

	template<class T>
	FSimpleIOStream& operator<<(const std::vector<T>& InValue)
	{
		*this << InValue.size();
		for (auto &Tmp:InValue)
		{
			Wirte(&Tmp, sizeof(T));
		}
		return *this;
	}

	template<class T>
	FSimpleIOStream& operator>>(std::vector<T>& InValue)
	{
		int Size = 0;
		*this >> Size;

		for (int i=0;i<Size;i++)
		{
			InValue.push_back(T());
			T& Instance = InValue.back();
			*this >> Instance;
		}
		return *this;
	}
};