#include "stdafx.h"
#include "CustomHeader.h"

using namespace std;

enum class PacketType
{
	First,
	Second
};


struct Packet
{
	int option;
	int pp;
};

void Input(Socket& sock)
{
	char message[1024] = { 0, };

	while (true)
	{
		scanf("%s", &message);

		//sock.m_receiveBuffer

		printf("%s", message);
	}

	
	//sock->OverlapWSAsend(*sock);
}

void ForPrint()
{
	for (int i = 0; i < 10; i++)
	{
		printf("%d\n", i);
	}
}

int main()
{
	try
	{
#ifdef _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw("WSAStartup Fail");
		}
#endif

		char message[1024] = { 0, };

		Iocp iocp(1);

		EndPoint serverEndPoint("192.168.55.52", 4444);

		Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		iocp.Add(client1, &client1);

		scanf("%s", &message);

		cout << client1.OverlapConnectEx(client1, serverEndPoint) << endl;
		cout << GetLastErrorAsString() << endl;
		/*std::thread iocpThread(Input);

		std::thread forThread(ForPrint);

		iocpThread.join();
		forThread.join();*/
		Sleep(500);

		client1.OverlapWSAsend(client1, &message);

		//send(client1.GetHandle(), (const char*)&message, sizeof(message), 0);

		cout << GetLastErrorAsString() << endl;

		//recv(client1.GetHandle(), (char*)&client1.m_overlappedStruct.m_buffer, sizeof(client1.m_overlappedStruct.m_buffer), 0);

		//printf("%s\n", client1.m_overlappedStruct.m_buffer);

		while (true)
		{
			IocpEvents iocpEvents;

			iocp.Wait(iocpEvents, 100);

			IOType ioType;

			for (int i = 0; i < iocpEvents.m_eventCount; ++i)
			{
				auto& iocpEvent = iocpEvents.m_events[i];

				ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

				if (ioType == IOType::READ)
				{

				}
				else if (ioType == IOType::WRITE)
				{
					delete iocpEvent.lpOverlapped;
					//delete[] ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_wsaBuf.buf;
				}
			}

		}

	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	//std::cout << Text("00000") << std::endl;

	return 0;
}