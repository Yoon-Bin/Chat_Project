#include "stdafx.h"
#include "CustomHeader.h"
#include "PacketProcess.h"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"


#define SIZE 0
#define TYPE 1

using namespace std;

static std::map<PacketType, void(*)(Socket*)> callbackmap;

void Input(Socket* sock)
{
	while (true)
	{
		string message;

		std::getline(cin, message);

		if (message.length() != 0)
		{
			unsigned short packetSize = sizeof(Packet_Chat) + sizeof(char) * message.length() + 1;

			Packet_Chat* packet = (Packet_Chat*)malloc(packetSize);
			
			if (packet != NULL)
			{
				packet->size = static_cast<char>(packetSize);
				packet->type = static_cast<char>(PacketType::CHAT);
				packet->id = sock->m_id;

				strcpy(packet->message, message.c_str());

				sock->OverlapWSAsend(packet);

				free(packet);
			}
		}
	}
}

void Ovlp(Iocp* iocp)
{
	while (true)
	{
		IocpEvents iocpEvents;

		iocp->Wait(iocpEvents, 100);

		for (int i = 0; i < iocpEvents.m_eventCount; ++i)
		{
			auto& iocpEvent = iocpEvents.m_events[i];

			IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

			switch (ioType)
			{
			case IOType::READ:
			{
				if (iocpEvent.dwNumberOfBytesTransferred <= 0)
				{
					printf("READ 0 byte\n");
				}
				
				Socket* sockPtr = (Socket*)iocpEvent.lpCompletionKey;

				PacketType packetType = static_cast<PacketType>(sockPtr->m_overlappedStruct.m_buffer[TYPE]);

				callbackmap[packetType](sockPtr);

				//printf("%s\n", sock->m_receiveBuffer);

				//RtlZeroMemory(sock->m_overlappedStruct.m_buffer, sizeof(sock->m_overlappedStruct.m_buffer))

				sockPtr->OverlapWSArecv();

				break;
			}
			case IOType::WRITE:
			{
				if (iocpEvent.dwNumberOfBytesTransferred <= 0)
				{
					printf("WRITE 0 byte\n");
				}

				delete iocpEvent.lpOverlapped;

				break;
			}
			case IOType::CONNECT:
			{
				Socket* sockPtr = (Socket*)iocpEvent.lpCompletionKey;

				if (iocpEvent.dwNumberOfBytesTransferred <= 0)
				{
					printf("CONNECT 0 byte\n");
				}
				sockPtr->OverlapWSArecv();


				break;

			}
			case IOType::DISCONNECT:
			{
				if (iocpEvent.dwNumberOfBytesTransferred <= 0)
				{
					printf("DISCONNECT 0 byte\n");
				}

				break;

			}
			}
		}
	}
}



auto MakePacket_Login_Request = [](Socket* sockPtr, string username, string password) {

	unsigned short packetSize = sizeof(Packet_Login_Request);

	Packet_Login_Request packet;

	packet.size = static_cast<char>(packetSize);
	packet.type = static_cast<char>(PacketType::LOGIN_REQUEST);
	strcpy(packet.username, username.c_str());
	strcpy(packet.password, password.c_str());

	sockPtr->OverlapWSAsend(&packet);
};

void Test(EndPoint* ePoint)
{
	for (int i = 0; i < 3; i++)
	{
		Socket sock(SockType::TCP);

		sock.Bind(EndPoint::Any);

		sock.OverlapConnectEx(ePoint);

		Sleep(300);
	}
}

//struct Client
//{
//	Socket sock;
//
//	char m_buffer[10] = "abcdefg";
//
//	std::thread* sockThread = new std::thread([&]() {
//
//		while (true)
//		{
//			sock.OverlapWSAsend(m_buffer);
//		}
//	});
//};



int main()
{
	callbackmap.insert({ PacketType::CHAT, PacketProcess_Chat_Print });
	callbackmap.insert({ PacketType::LOGIN_REPLY, PacketProcess_Login_Reply });
	callbackmap.insert({ PacketType::CRERATE_ACCOUNT_REPLY, PacketProcess_Create_Account_Reply });

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

		Iocp iocp(1);

		/*Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		iocp.Add(&client1);

		Socket client2(SockType::TCP);

		client2.Bind(EndPoint::Any);

		iocp.Add(&client2);*/

		/*std:vector<std::shared_ptr<thread>> threads;

		std::shared_ptr<thread> ovlpThread1(new std::thread(Test, &serverEndPoint));
		std::shared_ptr<thread> ovlpThread2(new std::thread(Test, &serverEndPoint));
		std::shared_ptr<thread> ovlpThread3(new std::thread(Test, &serverEndPoint));*/

		/*threads.push_back(ovlpThread1);
		threads.push_back(ovlpThread1);
		threads.push_back(ovlpThread1);*/
		//std::shared_ptr<thread> test(new std::thread(Test, &iocp, &serverEndPoint));

	
		//threads.push_back(test);

		//client1.OverlapConnectEx(&serverEndPoint);
		//client2.OverlapConnectEx(&serverEndPoint);

		boost::random::mt19937 gen;
		boost::random::uniform_int_distribution<> dist(0, 99);

		Packet_Chat packet;
		char buffer[10] = "abcdefg";

		packet.size = sizeof(Packet_Chat) + sizeof(buffer);
		packet.type = static_cast<char>(PacketType::CHAT);
		strcpy(packet.message, buffer);

		std::stringstream ss;

		ss << 3<< static_cast<char>(PacketType::CHAT) << buffer;

		std::cout << ss.str().c_str() << std::endl;

		std::atomic<bool> sendGuard = true;

		SocketPool pool1(100, true, SockType::TCP);
		SocketPool pool2(100, true, SockType::TCP);

		for (auto& sock : pool1.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		for (auto& sock : pool2.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}

		sendGuard.store(false, memory_order_release);

		std::thread *test1 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomNum = dist(gen);
					
					send(pool1.m_fullSockPtrVector[randomNum]->m_handle, ss.str().c_str(), 20, 0);
					
					Sleep(200);

					//sockets[0]->OverlapWSAsend(&packet);
				}
			}
		});
		std::thread* test2 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomNum = dist(gen);

					send(pool2.m_fullSockPtrVector[randomNum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(200);

					//sockets[0]->OverlapWSAsend(&packet);
				}
			}
			});
		test1->join();
		test2->join();

		/*for (auto thread : threads)
		{
			thread->join();
		}*/
		//Sleep(2000);

		//client1.OverlapDisconnectEx();

		

		//printf("Write Your ID & Password\n");
		//printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

		//string username;
		//string password;

		//WriteUserInfo(username, "Username");
		//WriteUserInfo(password, "Password");

		//MakePacket_Login_Request(&client1, username, password);

		////std::thread chatThread(Input, &client1);

		////std::thread forThread(ForPrint);
		//chatThread.join();
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

		//closesocket(client1.m_handle);
	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}