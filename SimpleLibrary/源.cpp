
#include "simple_library/public/simple_library.h"
DEFINITION_MULTICAST_SINGLE_DELEGATE(TestDD, void, int)


//using namespace std;

int main()
{
#if 0
	TestDD dd;
	auto H = dd.AddFunction([](int a)
		{
			cout << "Hello" << endl;
		});

	dd.Broadcast(1);
	dd.RemoveDelegate(H);
#endif
	const char LogPath[] = "../Log";
	init_log_system(LogPath);
	log_log("Log Init");

	FSimpleNetDrive* TCPServer = FSimpleNetDrive::GetNetDrive(ESimpleSocketType::SIMPLETYPE_TCP, ESimpleDriveType::DRIVETYPE_LISTEN);
	FSimpleNetDrive* TCPClient = FSimpleNetDrive::GetNetDrive(ESimpleSocketType::SIMPLETYPE_TCP, ESimpleDriveType::DRIVETYPE_CONNECTION);

	if (!TCPServer->Init())
	{
		char Inlog[] = "TCPServer->Init()";
		//char F[] = "%s";
		//log_error(F, Inlog);
		log_error("%s",Inlog);
		return -1;
	}
	if (!TCPClient->Init())
	{
		
		log_error("%s","TCPClient->Init()");
		return -1;
	}

	auto LastTime = std::chrono::steady_clock::now();
	while (true)
	{
		Sleep(300);//·ÀÖ¹Õ³°ü

		auto CurrentTime =std::chrono::steady_clock::now();
		auto TimeInterval = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastTime);
		double TimeIntervalValue = double(TimeInterval.count()) / 100;
		//printf("%f\n", TimeIntervalValue);
		TCPClient->Tick(TimeIntervalValue);
		//Sleep(300);
		TCPServer->Tick(TimeIntervalValue);
		
		LastTime = CurrentTime;
	}


	return 0;
}