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

void Input(Socket* sock)
{
	while (true)
	{
		char message[1024] = { 0, };

		scanf("%s", &message);

		//sock.m_receiveBuffer

		sock->OverlapWSAsend(&message);
	}


	//sock->OverlapWSAsend(*sock);
}

void Ovlp(Socket* sock, Iocp* iocp)
{
	while (true)
	{
		IocpEvents iocpEvents;

		iocp->Wait(iocpEvents, 100);

		IOType ioType;

		for (int i = 0; i < iocpEvents.m_eventCount; ++i)
		{
			auto& iocpEvent = iocpEvents.m_events[i];

			ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

			if (ioType == IOType::READ)
			{
				printf("%s\n", sock->m_receiveBuffer);

				RtlZeroMemory(sock->m_receiveBuffer, sizeof(sock->m_receiveBuffer));

				sock->OverlapWSArecv();
			}
			else if (ioType == IOType::WRITE)
			{
				delete iocpEvent.lpOverlapped;
				//delete[] ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_wsaBuf.buf;
			}
		}
		//printf("d\n");
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
		/*char message[1024] = { 0, };

		scanf("%s", &message);*/

		EndPoint serverEndPoint("192.168.55.52", 4444);

		Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		Iocp iocp(1);

		iocp.Add(client1, &client1);

		cout << client1.OverlapConnectEx(client1, serverEndPoint) << endl;
		cout << GetLastErrorAsString() << endl;
		client1.OverlapWSArecv();

		std::thread ovlpThread(Ovlp, &client1, &iocp);
		std::thread chatThread(Input, &client1);

		//std::thread forThread(ForPrint);
		ovlpThread.join();
		chatThread.join();
		//forThread.join();

		//sock.m_receiveBuffer

		//client1.OverlapWSAsend(&message);

		//while (true)
		//{
		//	IocpEvents iocpEvents;

		//	iocp.Wait(iocpEvents, 100);

		//	IOType ioType;

		//	for (int i = 0; i < iocpEvents.m_eventCount; ++i)
		//	{
		//		auto& iocpEvent = iocpEvents.m_events[i];

		//		ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

		//		if (ioType == IOType::READ)
		//		{
		//			printf("%s\n", client1.m_receiveBuffer);

		//			RtlZeroMemory(client1.m_receiveBuffer, sizeof(client1.m_receiveBuffer));

		//			client1.OverlapWSArecv();
		//		}
		//		else if (ioType == IOType::WRITE)
		//		{
		//			delete iocpEvent.lpOverlapped;
		//			//delete[] ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_wsaBuf.buf;
		//		}
		//	}
		//}

		//client1.OverlapWSAsend( &message);
		//send(client1.GetHandle(), (const char*)&message, sizeof(message), 0);


		//recv(client1.GetHandle(), (char*)&client1.m_overlappedStruct.m_buffer, sizeof(client1.m_overlappedStruct.m_buffer), 0);

		//printf("%s\n", client1.m_overlappedStruct.m_buffer);


	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	//std::cout << Text("00000") << std::endl;

	return 0;
}