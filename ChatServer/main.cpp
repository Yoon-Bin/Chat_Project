#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"
#include "PacketFunction_Server.h"

#define SIZE 0
#define TYPE 1

static std::map<PacketType, void(*)(const Socket* const)> callbackmap;

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

		printf("%s\n\n", mysql_get_client_info());
		callbackmap.insert({ PacketType::CHAT, S2C::Chat_Reply });
		callbackmap.insert({ PacketType::LOGIN_REQUEST, S2C::Login_Reply });
		callbackmap.insert({ PacketType::CRERATE_ACCOUNT_REQUEST, S2C::Create_Account_Reply });

		bool setTrue	= true;
		bool setFalse	= false;

		EndPoint endPoint("192.168.55.52", 4444);
		Socket listenSock(SockType::TCP);

		Iocp iocp(0);

		SocketPool sockPool(10, false, SockType::TCP);

		std::cout << "listenSock : " << &listenSock << std::endl;

		listenSock.Bind(endPoint);

		listenSock.SetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, setTrue);
		//listenSock.SetSockOpt(SOL_SOCKET, SO_REUSEADDR, setTrue);

		listenSock.GetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT);
		//listenSock.GetSockOpt(SOL_SOCKET, SO_REUSEADDR);

		listenSock.Listen(1000);

		iocp.Add(&listenSock);

		for (auto& iterator : sockPool.m_fullSockPtrVector)
		{
			//왜 REUSEADDR를 안 써도 재사용이 되지?
			//iterator->SetSockOpt(SOL_SOCKET, SO_REUSEADDR, setTrue);
			//iterator->GetSockOpt(SOL_SOCKET, SO_REUSEADDR);

			iocp.Add(iterator.get());

			listenSock.OverlapAcceptEx(iterator.get());
		}

		std::cout << sockPool.GetFullSockCount() << " Sockets OverlapAccepted" << std::endl;

		std::vector<std::shared_ptr<std::thread>> threads;

		for (int i = 0; i < 2; i++)
		{
			std::shared_ptr<std::thread> ovlpThread(new std::thread([&iocp, &sockPool, &listenSock]() {

				while (true)
				{
					IocpEvents iocpEvents;
					iocp.Wait(iocpEvents, 0);

					for (int i = 0; i < iocpEvents.m_eventCount; ++i)
					{
						auto& iocpEvent = iocpEvents.m_events[i];

						IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

						/*if (iocpEvent.dwNumberOfBytesTransferred <= 0)
						{
							printf("0 byte\n");
						}*/

						switch (ioType)
						{
						case IOType::ACCEPT:
						{
							unsigned short sockID = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_id;

							Socket* sockPtr = sockPool.m_fullSockPtrVector[sockID].get();

							sockPtr->UpdateAcceptContext(listenSock);

							sockPtr->OverlapWSArecv();

							sockPtr->m_isOverlapped = true;

							printf("%d\n", (int)sockPtr->m_handle);

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
				}));

			threads.push_back(ovlpThread);
		}

		for (auto thread : threads)
		{
			thread->join();
		}

	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}
	
	WSACleanup();
	return 0;
}