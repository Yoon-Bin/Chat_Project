#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"
//extern SocketUtil sockUtil;

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
		Iocp iocp(1);
		SocketPool sockPool(50);

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

		iocp.Add(listenSock, nullptr);

		for (auto iterator : sockPool.m_sockPool)
		{
			listenSock.m_overlappedStruct.m_ioType = IOType::ACCEPT;

			printf("%p\n", iterator.second.get());

			listenSock.OverlapAcceptEx(iterator.second.get());

			printf("%d\n", iterator.second->GetHandle());

			sockPool.Insert(iterator.second.get());
		}
		
		mutex m1;
		
		while (true)
		{
			IocpEvents iocpEvents;
			iocp.Wait(iocpEvents, 100);

			for (int i = 0; i < iocpEvents.m_eventCount; ++i)
			{
				auto& iocpEvent = iocpEvents.m_events[i];
				
				IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

				//lock_guard<mutex> lockguard(m1);
				if (ioType == IOType::ACCEPT)
				{
					Socket* usableSock = sockPool.GetUsableSock();
					
					sockPool.Pop();

					sockPool.m_sockPool[usableSock]->UpdateAcceptContext(listenSock);
					
					iocp.Add(*usableSock, usableSock);

					sockPool.m_sockPool[usableSock]->OverlapWSArecv();

				}
				else if (ioType == IOType::READ)
				{
					shared_ptr<Socket> sock = sockPool.m_sockPool[(Socket*)iocpEvent.lpCompletionKey];

					cout << "read" << endl;
					if (iocpEvent.dwNumberOfBytesTransferred > 0)
					{
						printf("%d\n", iocpEvent.dwNumberOfBytesTransferred);
						printf("%s\n", sock->m_receiveBuffer);
					}

					sock->OverlapWSAsend(*sock);
				}
				else if (ioType == IOType::WRITE)
				{
					cout << "write" << endl;
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