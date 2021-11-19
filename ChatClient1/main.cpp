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

auto MakePacket_Login_Request = [](Socket* sockPtr, string username, string password) {

	unsigned short packetSize = sizeof(Packet_Login_Request);

	Packet_Login_Request packet;

	packet.size = static_cast<char>(packetSize);
	packet.type = static_cast<char>(PacketType::LOGIN_REQUEST);
	strcpy(packet.username, username.c_str());
	strcpy(packet.password, password.c_str());

	sockPtr->OverlapWSAsend(&packet);
};

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
		EndPoint serverEndPoint("192.168.55.52", 4444);

		Iocp iocp(1);

		Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		iocp.Add(&client1);

		client1.OverlapConnectEx(&serverEndPoint);

		printf("Write Your ID & Password\n");
		printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

		string username;
		string password;

		WriteUserInfo(username, "Username");
		WriteUserInfo(password, "Password");

		MakePacket_Login_Request(&client1, username, password);

		std::thread chatThread(Input, &client1);

		//std::thread forThread(ForPrint);
		chatThread.join();
		//forThread.join();

		//sock.m_receiveBuffer

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
				case IOType::CONNECT:
				{
					client1.OverlapWSArecv();

					break;
				}
				case IOType::READ:
				{
					if (iocpEvent.dwNumberOfBytesTransferred > 0)
					{
						client1.m_isOverlapped = false;

						PacketType packetType = static_cast<PacketType>(client1.m_overlappedStruct.m_buffer[TYPE]);

						callbackmap[packetType](&client1);

						client1.OverlapWSArecv();

						client1.m_isOverlapped = true;
					}

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
	

		//client1.OverlapWSAsend( &message);
		//send(client1.GetHandle(), (const char*)&message, sizeof(message), 0);


		//recv(client1.GetHandle(), (char*)&client1.m_overlappedStruct.m_buffer, sizeof(client1.m_overlappedStruct.m_buffer), 0);

		//printf("%s\n", client1.m_overlappedStruct.m_buffer);

		//closesocket(client1.m_handle);


		///////////////////
		//Multi Threading//
		///////////////////
		/*
		boost::random::mt19937 gen;
		boost::random::uniform_int_distribution<> dist(0, 99);

		Packet_Chat packet;
		char buffer[10] = "abcdefg";

		packet.size = sizeof(Packet_Chat) + sizeof(buffer);
		packet.type = static_cast<char>(PacketType::CHAT);
		strcpy(packet.message, buffer);

		std::stringstream ss;

		ss << 3 << static_cast<char>(PacketType::CHAT) << buffer;

		std::cout << ss.str().c_str() << std::endl;

		std::atomic<bool> sendGuard = true;

		SocketPool pool1(100, true, SockType::TCP);
		SocketPool pool2(100, true, SockType::TCP);
		SocketPool pool3(100, true, SockType::TCP);
		SocketPool pool4(100, true, SockType::TCP);
		SocketPool pool5(100, true, SockType::TCP);

		for (auto& sock : pool1.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		for (auto& sock : pool2.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		for (auto& sock : pool3.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		for (auto& sock : pool4.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		for (auto& sock : pool5.m_fullSockPtrVector)
		{
			sock->Connect(serverEndPoint);
		}
		printf("aaaaaaaaaaaaaaaaaa\n");
		sendGuard.store(false, memory_order_release);

		std::thread* test1 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomnum = dist(gen);

					send(pool1.m_fullSockPtrVector[randomnum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(10);
				}
			}
			});
		std::thread* test2 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomnum = dist(gen);

					send(pool2.m_fullSockPtrVector[randomnum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(10);
				}
			}
			});
		std::thread* test3 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomnum = dist(gen);

					send(pool3.m_fullSockPtrVector[randomnum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(10);
				}
			}
			});
		std::thread* test4 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomnum = dist(gen);

					send(pool4.m_fullSockPtrVector[randomnum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(10);
				}
			}
			});
		std::thread* test5 = new thread([&]() {

			if (!sendGuard.load(memory_order_acquire))
			{
				while (true)
				{
					int randomnum = dist(gen);

					send(pool5.m_fullSockPtrVector[randomnum]->m_handle, ss.str().c_str(), 20, 0);

					Sleep(10);
				}
			}
			});

		std::vector <std::thread*> threads;

		threads.push_back(test1);
		threads.push_back(test2);
		threads.push_back(test3);
		threads.push_back(test4);
		threads.push_back(test5);

		for (auto thread : threads)
		{
			thread->join();
		}*/
	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	return 0;
}