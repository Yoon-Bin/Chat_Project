#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"

#define SIZE 0
#define TYPE 1

using namespace std;

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
		//DB Test Code
		printf("%s\n\n", mysql_get_client_info());

		std::map<PacketType, void(*)(Socket*)> callbackmap;

		auto PacketProcess_Chat = [](Socket* sockPtr) {
			
			Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

			printf("%d : %s\n", packet->id, packet->message);

			sockPtr->OverlapWSAsend(packet);
		};	
		callbackmap.insert({ PacketType::CHAT, PacketProcess_Chat });



		Iocp iocp(1);
		SocketPool sockPool(10);

		bool setTrue	= true;
		bool setFalse	= false;

		EndPoint endPoint("192.168.55.52", 4444);

		Socket listenSock(SockType::TCP);

		listenSock.Bind(endPoint);

		listenSock.SetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, setTrue);
		listenSock.SetSockOpt(SOL_SOCKET, SO_EXCLUSIVEADDRUSE, setTrue);

		listenSock.GetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT);
		listenSock.GetSockOpt(SOL_SOCKET, SO_EXCLUSIVEADDRUSE);

		listenSock.Listen(1000);

		iocp.Add(&listenSock);

		for (auto& iterator : sockPool.m_fullSockPtrMap)
		{
			listenSock.OverlapAcceptEx(iterator.second.get());

			sockPool.PushUsableSockPtr(iterator.second.get());
		}

		if (sockPool.GetFullSockCount() == sockPool.GetUsableSockCount())
		{
			cout << sockPool.GetUsableSockCount() << " Sockets OverlapAccepted" << endl;
		}
		
		//mutex m1;
		
		while (true)
		{
			IocpEvents iocpEvents;
			iocp.Wait(iocpEvents, 100);

			for (int i = 0; i < iocpEvents.m_eventCount; ++i)
			{
				auto& iocpEvent = iocpEvents.m_events[i];
				
				IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

				switch (ioType)
				{
				case IOType::ACCEPT:
				{
					Socket* sockPtr = sockPool.GetUsableSockPtr();

					sockPtr->UpdateAcceptContext(&listenSock);

					iocp.Add(sockPtr);

					sockPtr->OverlapWSArecv();

					sockPool.PopUsableSockPtr();

					break;
				}
				case IOType::READ:
				{
					Socket* sockPtr = (Socket*)iocpEvent.lpCompletionKey;

					PacketType packetType = static_cast<PacketType>(sockPtr->m_overlappedStruct.m_buffer[TYPE]);

					callbackmap[packetType](sockPtr);

					sockPtr->OverlapWSArecv();

					break;
				}
				case IOType::WRITE:
				{
					break;
				}

				}
			}
		}
	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}
	
	return 0;
}