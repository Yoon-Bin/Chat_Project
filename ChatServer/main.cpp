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

		auto PacketProcess_Login_Request = [](Socket* sockPtr) {
			
			Packet_Login_Request* packet = reinterpret_cast<Packet_Login_Request*>(sockPtr->m_overlappedStruct.m_buffer);

			MYSQL conn;
			MYSQL* connPtr = nullptr;
			MYSQL_RES* result;
			MYSQL_ROW row;
			char query[128];
			int stat;

			RtlZeroMemory(query, 128);

			mysql_init(&conn);

			connPtr = mysql_real_connect(&conn, "127.0.0.1", "root", "tjqjvmfajyb12#", "sampledatabase", 3306, (char*)NULL, 0);

			if (connPtr == NULL)
			{
				fprintf(stderr, "Mysql Connection Error : %s\n", mysql_error(&conn));
				//return 1;
			}
			
			std::strcat(query, "select password from user where username='");
			std::strcat(query, packet->username);
			std::strcat(query, "'");

			stat = mysql_query(&conn, query);

			if (stat != 0)
			{
				fprintf(stderr, "Mysql Err : %s \n", mysql_error(&conn));
				//return 1;
			}

			result = mysql_store_result(connPtr);
			
			row = mysql_fetch_row(result);

			if (row == nullptr)
			{
				std::cout << "There isn't Accout" << std::endl;
			}
			else
			{
				Packet_Login_Reply replyPacket;

				replyPacket.size = static_cast<char>(sizeof(Packet_Login_Reply));
				replyPacket.type = static_cast<char>(PacketType::LOGIN_REPLY);
				replyPacket.id = static_cast<unsigned short>(sockPtr->m_handle);

				if (strcmp(row[0], packet->password) == 0)
				{
					replyPacket.success = true;
				}
				else
				{
					replyPacket.success = false;
				}
				sockPtr->OverlapWSAsend(&replyPacket);
			}

			mysql_close(connPtr);
		};
		callbackmap.insert({ PacketType::LOGIN_REQUEST, PacketProcess_Login_Request });


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