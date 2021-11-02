#include "stdafx.h"
#include "CustomHeader.h"

using namespace std;

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

void Ovlp(Socket* sock, Iocp* iocp)
{
	while (true)
	{
		IocpEvents iocpEvents;

		iocp->Wait(iocpEvents, 100);

		for (int i = 0; i < iocpEvents.m_eventCount; ++i)
		{
			auto& iocpEvent = iocpEvents.m_events[i];

			IOType ioType = ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_ioType;

			if (ioType == IOType::READ)
			{
				if ((PacketType)sock->m_overlappedStruct.m_buffer[1] == PacketType::CHAT)
				{
					Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sock->m_overlappedStruct.m_buffer);

					printf("%s\n", packet->message);
				}
				else if ((PacketType)sock->m_overlappedStruct.m_buffer[1] == PacketType::LOGIN_REPLY)
				{
					Packet_Login_Reply* packet = reinterpret_cast<Packet_Login_Reply*>(sock->m_overlappedStruct.m_buffer);
					sock->m_id = packet->id;
				}

				//printf("%s\n", sock->m_receiveBuffer);

				RtlZeroMemory(sock->m_overlappedStruct.m_buffer, sizeof(sock->m_overlappedStruct.m_buffer));

				sock->OverlapWSArecv();
			}
			else if (ioType == IOType::WRITE)
			{
				delete iocpEvent.lpOverlapped;
				//delete[] ((OverlappedStruct*)iocpEvent.lpOverlapped)->m_wsaBuf.buf;
			}
		}
		//printf("d\n");
	}
}

auto WriteUserInfo = [](string& info, const char* infoType)
{
	while (true)
	{
		printf("%s : ", infoType);
		std::getline(cin, info);

		if (info.length() > MAX_USERINFO_SIZE) {
			printf("Too Long, Write Again\n\n");
			continue;
		}
		else
			break;
	}
};

auto MakePacket_Login_Request = [](Socket* sockPtr, string username, string password) {

	unsigned short packetSize = sizeof(Packet_Login_Request);

	Packet_Login_Request packet;

	packet.size = static_cast<char>(packetSize);
	packet.type = static_cast<char>(PacketType::LOGIN_REQUEST);
	strcpy(packet.username, username.c_str());
	strcpy(packet.password, password.c_str());

	sockPtr->OverlapWSAsend(&packet);
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
		/*char message[1024] = { 0, };

		scanf("%s", &message);*/

		EndPoint serverEndPoint("192.168.55.52", 4444);

		Socket client1(SockType::TCP);

		client1.Bind(EndPoint::Any);

		Iocp iocp(1);

		iocp.Add(&client1);

		client1.OverlapConnectEx(&serverEndPoint);

		client1.OverlapWSArecv();

		std::thread ovlpThread(Ovlp, &client1, &iocp);

		printf("Write Your ID & Password\n");
		printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

		string username;
		string password;

		WriteUserInfo(username, "Username");
		WriteUserInfo(password, "Password");

		MakePacket_Login_Request(&client1, username, password);

		std::thread chatThread(Input, &client1);

		//std::thread forThread(ForPrint);
		ovlpThread.join();
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

		
	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	//std::cout << Text("00000") << std::endl;

	return 0;
}