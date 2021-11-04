#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"
#include "PacketProcess.h"

#define SIZE 0
#define TYPE 1

//using namespace std;

static std::map<PacketType, void(*)(Socket*)> callbackmap;

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

		callbackmap.insert({ PacketType::CHAT, PacketProcess_Chat_Resend });
		callbackmap.insert({ PacketType::LOGIN_REQUEST, PacketProcess_Login_Request });
		callbackmap.insert({ PacketType::CRERATE_ACCOUNT_REQUEST, PacketProcess_Create_Account_Request });

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
			std::cout << sockPool.GetUsableSockCount() << " Sockets OverlapAccepted" << std::endl;
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
					delete iocpEvent.lpOverlapped;

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