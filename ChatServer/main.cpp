#include "stdafx.h"
#include "Iocp.h"
#include "SocketPool.h"
//extern SocketUtil sockUtil;

using namespace std;

struct Test
{
	int length;
	int id;
	int a;
	char message[];
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
		SocketPool sockPool(10);

		std::unordered_set<Socket*> connectedSockets;

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

			//printf("%p\n", iterator.second.get());

			listenSock.OverlapAcceptEx(iterator.second.get());

			//printf("%d\n", iterator.second->GetHandle());

			sockPool.Insert(iterator.second.get());
		}

		cout << sockPool.GetSize() << " Sockets OverlapAccepted" << endl;
		
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

					connectedSockets.insert(usableSock);

					sockPool.m_sockPool[usableSock]->OverlapWSArecv();

				}
				else if (ioType == IOType::READ)
				{
					shared_ptr<Socket> sock = sockPool.m_sockPool[(Socket*)iocpEvent.lpCompletionKey];

					cout << "read" << endl;
					if (iocpEvent.dwNumberOfBytesTransferred > 0)
					{
						if ((int)sock->m_receiveBuffer[1] == PacketType::CHAT)
						{
							printf("%d\n", iocpEvent.dwNumberOfBytesTransferred);
							printf("%d\n", (int)sock->m_receiveBuffer[0]);
							printf("%d\n", (int)sock->m_receiveBuffer[1]);
							printf("%d\n", (int)sock->m_receiveBuffer[2]);
							//std::cout << sock->m_receiveBuffer[3] << std::endl;
							Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sock->m_receiveBuffer);
							//Packet_Chat packet = (Packet_Chat)sock->m_
							
							printf("%s\n", packet->message);
						}
						
						//printf("%s\n", packet);

					}
					sock->OverlapWSArecv();

					for (auto iterator : connectedSockets)
					{
						iterator->OverlapWSAsend(&(sock->m_receiveBuffer));
					}
					RtlZeroMemory(sock->m_receiveBuffer, sizeof(sock->m_receiveBuffer));

					//sock->OverlapWSAsend(&(sock->m_receiveBuffer));
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