#include "stdafx.h"
#include "CustomHeader.h"
#include "PacketFunction_Client.h"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/lockfree/queue.hpp"

#include <future>

#define SIZE 0
#define TYPE 1

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

		Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		iocp.Add(&client1);

		std::shared_ptr<std::thread> ovlpThread(new std::thread([&iocp, &client1]() {

			while (true)
			{
				IocpEvents iocpEvents;
				iocp.Wait(iocpEvents, 0);

				for (int i = 0; i < iocpEvents.m_eventCount; ++i)
				{
					auto& iocpEvent = iocpEvents.m_events[i];

					IOType& ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

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

							PacketProcess(client1);

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
			}));

		client1.OverlapConnectEx(EndPoint::EndPoint("192.168.55.52", 4444));

		ProcessLogin(client1);

		std::vector<std::shared_ptr<std::thread>> threadVector;

		threadVector.push_back(ovlpThread);

		for (auto th : threadVector)
		{
			th->join();
		}

		


		/******************/
		//Multiple Clients//
		/******************/

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