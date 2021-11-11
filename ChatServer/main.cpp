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

		std::cout << "listenSock : " << &listenSock << std::endl;

		listenSock.Bind(endPoint);

		listenSock.SetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, setTrue);
		//listenSock.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, setTrue);

		listenSock.GetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT);
		//listenSock.GetSockOpt(SOL_SOCKET, SO_REUSEADDR);

		listenSock.Listen(1000);

		iocp.Add(&listenSock);

		for (auto& iterator : sockPool.m_fullSockPtrMap)
		{
			//�� REUSEADDR�� �� �ᵵ ������ ����?
			//iterator->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, setTrue);
			//iterator->GetSockOpt(SOL_SOCKET, SO_REUSEADDR);

			iocp.Add(iterator.get());

			listenSock.OverlapAcceptEx(iterator.get());
		}

		//sockPool.m_fullSockPtrMap[1].get()->OverlapAcceptEx(listenSock);
		//sockPool.m_fullSockPtrMap[2].get()->OverlapAcceptEx(listenSock);
		//sockPool.m_fullSockPtrMap[0].get()->OverlapAcceptEx(listenSock);

		std::cout << sockPool.GetFullSockCount() << " Sockets OverlapAccepted" << std::endl;

		//mutex m1;
		
		while (true)
		{
			IocpEvents iocpEvents;
			iocp.Wait(iocpEvents, 100);

			for (int i = 0; i < iocpEvents.m_eventCount; ++i)
			{
				auto& iocpEvent = iocpEvents.m_events[i];
				
				IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

				/*std::cout << (int)iocpEvent.dwNumberOfBytesTransferred << std::endl;

				if (iocpEvent.dwNumberOfBytesTransferred <= 0)
				{
					printf("0 byte\n");
				}*/

				switch (ioType)
				{
				case IOType::ACCEPT:
				{
					unsigned short sockID = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_id;

					Socket* sockPtr = sockPool.m_fullSockPtrMap[sockID].get();

					std::cout << sockPtr->m_handle << std::endl;

					sockPtr->UpdateAcceptContext(&listenSock);

					sockPtr->OverlapWSArecv();

					sockPtr->m_isOverlapped = true;

					break;
				}
				case IOType::READ:
				{
					Socket* sockPtr = (Socket*)iocpEvent.lpCompletionKey;

					if (iocpEvent.dwNumberOfBytesTransferred > 0)
					{
						sockPtr->m_isOverlapped = false;

						PacketType packetType = static_cast<PacketType>(sockPtr->m_overlappedStruct.m_buffer[TYPE]);

						callbackmap[packetType](sockPtr);

						sockPtr->OverlapWSArecv();

						sockPtr->m_isOverlapped = true;
					}
					else
					{
						sockPtr->OverlapDisconnectEx();
					}

					break;
				}
				case IOType::WRITE:
				{
					delete iocpEvent.lpOverlapped;

					break;
				}
				case IOType::DISCONNECT:
				{
					Socket* sockPtr = (Socket*)iocpEvent.lpCompletionKey;

					RtlZeroMemory(&sockPtr->m_overlappedStruct.m_wsaOverlapped, sizeof(sockPtr->m_overlappedStruct.m_wsaOverlapped));

					listenSock.OverlapAcceptEx(sockPtr);

					printf("%d Disconnected\n", (int)sockPtr->m_handle);
				}
				}
			}
		}
	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}
	
	WSACleanup();
	return 0;
}