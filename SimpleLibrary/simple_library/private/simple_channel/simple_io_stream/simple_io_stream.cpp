#pragma once
#include "../../../public/simple_channel/simple_core/simple_io_stream.h"

FSimpleIOStream::FSimpleIOStream(TArray<unsigned char>& InBuffer)
	:Buffer(InBuffer),
	Ptr(nullptr)
{

}

void FSimpleIOStream::Wirte(const void* InData, int Inlength)
{
	int Pos = Buffer.AddUninitialized(Inlength);
	memcpy(&Buffer[Pos], InData,Inlength);
}

void FSimpleIOStream::Seek(int InPos)
{
	if (!Ptr)
	{
		Ptr = Buffer.GetData();//获得首地址
	}
	Ptr += InPos;
}

unsigned char* FSimpleIOStream::Begin()
{
	Ptr = Buffer.GetData();
	return Ptr;
}

unsigned char* FSimpleIOStream::End()
{
	Begin();
	Ptr += Buffer.Num();
	return Ptr;
}

unsigned char* FSimpleIOStream::Tall()
{
	return Ptr;
}

FSimpleIOStream& FSimpleIOStream::operator<<(const std::string& InValue)
{
	Wirte(InValue.c_str(), InValue.size());
	Wirte("\0", 1);//结尾加入
	return *this;
}
//宽字符string
FSimpleIOStream& FSimpleIOStream::operator<<(const std::wstring& InValue)
{
	return *this;
}

FSimpleIOStream& FSimpleIOStream::operator>>(std::string& InValue)
{
	InValue = (char*)Ptr;
	Seek(InValue.size() + 1);//读取结尾的\0

	return *this;
}

FSimpleIOStream& FSimpleIOStream::operator>>(std::wstring& InValue)
{
	return *this;
}
