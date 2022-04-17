#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"
#include "PacketFunction_Server.h"
#include "Serializer.h"
#include <chrono>
#include <array>
#include <time.h>

#define SIZE 0
#define TYPE 1


int main()
{
	for (int i = 1; i <= MAXROOMCOUNT; i++)
	{
		roomList.insert(std::make_pair(i, std::make_shared<Room>(i)));
		deactivatedRoomList.push(i);
	}

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

		EndPoint endPoint("192.168.55.52", 4444);
		Socket listenSock(SockType::TCP);

		Iocp iocp(0);

		SocketPool sockPool(10, false, SockType::TCP);

		std::cout << "listenSock : " << &listenSock << std::endl;

		listenSock.Bind(endPoint);

		listenSock.SetSockOpt(SOL_SOCKET, SO_CONDITIONAL_ACCEPT, true);
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

			listenSock.OverlapAcceptEx(*iterator);
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

						IOType& ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

						/*if (iocpEvent.dwNumberOfBytesTransferred <= 0)
						{
							printf("0 byte\n");
						}*/

						switch (ioType)
						{
						case IOType::ACCEPT:
						{
							unsigned short sockID = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_id;

							Socket& sockRef = *sockPool.m_fullSockPtrVector[sockID];

							sockRef.UpdateAcceptContext(listenSock);

							sockRef.OverlapWSArecv();

							sockRef.m_isOverlapped = true;

							printf("%d\n", (int)sockRef.m_handle);

							break;
						}
						case IOType::READ:
						{
							Socket& sockRef = *(Socket*)iocpEvent.lpCompletionKey;

							if (iocpEvent.dwNumberOfBytesTransferred > 0)
							{
								sockRef.m_isOverlapped = false;

								//패킷 재조립 기능 필요
								PacketProcess(sockRef);

								sockRef.OverlapWSArecv();

								sockRef.m_isOverlapped = true;
							}
							else
							{
								sockRef.OverlapDisconnectEx();
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

							//참조로 변경 필요
							Socket& sockRef = *(Socket*)iocpEvent.lpCompletionKey;

							RtlZeroMemory(&sockRef.m_overlappedStruct.m_wsaOverlapped, sizeof(sockRef.m_overlappedStruct.m_wsaOverlapped));

							listenSock.OverlapAcceptEx(sockRef);

							printf("%d Disconnected\n", (int)sockRef.m_handle);
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