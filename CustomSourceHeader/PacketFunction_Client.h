#pragma once
#include "stdafx.h"
#include "Socket.h"

static bool chat;

auto CutBlankBehind = [](std::string& text) {

	for (std::string::reverse_iterator it = text.rbegin(); it != text.rend(); it++)
	{
		if (*it == ' ')
		{
			text.erase(it.base());
		}
		else
		{
			text.erase(it.base());
			break;
		}
	}
};


auto WriteUserInfo = [](std::string& username, std::string& password) {

	username.clear();
	password.clear();

	while (true)
	{
		printf("Username : ");
		std::getline(std::cin, username);
		
		if (username.length() > MAX_USERINFO_SIZE) {
			printf("Too Long, Write Again\n\n");
			continue;
		}
		
		printf("Password : ");
		std::getline(std::cin, password);

		if (password.length() > MAX_PASSWORD_SIZE) {
			printf("Too Long, Write Again\n\n");
			continue;
		}

		CutBlankBehind(username);
		CutBlankBehind(password);

		break;
	}
};



namespace C2S
{
	auto Login_Request = [](const Socket& sockRef, Serializer& serializerRef) {

		unsigned short packetSize = sizeof(Packet_Login_Request);

		Packet_Login_Request packet;

		//strcpy(packet.username, username.c_str());
		//strcpy(packet.password, password.c_str());

		//sockPtr->OverlapWSAsend(&packet);
	};

	auto Create_Account_Request = [](const Socket* const sockPtr, const std::string username, const std::string password) {
		
		Packet_Create_Account_Request packet;

		strcpy(packet.username, username.c_str());
		strcpy(packet.password, password.c_str());

		sockPtr->OverlapWSAsend(&packet);
	};

	auto ProcessLogin = [](const Socket* const sockPtr) {

		printf("Write Your ID & Password\n");
		printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

		std::string username;
		std::string password;

		WriteUserInfo(username, password);

		Serializer serializer;

		serializer << username.c_str();
		serializer << password.c_str();
		
		serializer.SetHeader(PacketType::LOGIN_REQUEST);
		sockPtr->OverlapWSAsend(serializer);
	//Room Create, Exit Request
	auto Room_Simple_Request = [](const Socket& sockRef, const PacketType& type) {

		Serializer se;

		se.SetHeader(type);

		sockRef.OverlapWSAsend(se);
	};

	auto Room_Enter_Request = [](const Socket& sockRef, unsigned short roomID) {

		Serializer se;

		se << roomID;

		se.SetHeader(PacketType::ROOM_ENTER_REQUEST);

		sockRef.OverlapWSAsend(se);
	};

	auto Room_Info_Request = [](const Socket& sockRef, unsigned short page) {

		Serializer se;

		se << page;

		se.SetHeader(PacketType::ROOM_INFO_REQUEST);

		sockRef.OverlapWSAsend(se);
	};

	auto ProcessRoom = [](const Socket& sockRef) {

		std::shared_ptr<std::thread> roomThread(new std::thread([&sockRef]() {

			printf("\n1. RoomInfo\n2. EnterRoom\n\n");
			while (true)
			{
				char answer;

				answer = _getwch();

				int number;

				if (answer == '1')
				{
					printf("Type the number(Room Page Index)\n");

					while (true)
					{
						std::cin >> number;
						std::cin.ignore();
						if (number <= 0)
						{
							continue;
						}
						else
						{
							C2S::Room_Info_Request(sockRef, number - 1);
							break;
						}

					}
				}
				else if (answer == '2')
				{
					printf("Type the number(Room Index To Join)\n");

					while (true)
					{
						std::cin >> number;
						std::cin.ignore();
						if (number <= 0)
						{
							continue;
						}
						else
						{
							C2S::Room_Enter_Request(sockRef, number);
							break;
						}
					}
					break;
				}
				else
				{
					printf("Wrong Number\n");
					continue;
				}
			}
			}));

		roomThread->detach();
	};
}

namespace S2C
{
	auto Login_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		//Packet_Login_Reply* packet = reinterpret_cast<Packet_Login_Reply*>(sockPtr->m_overlappedStruct.m_buffer);

		unsigned short id;
		bool success;
		bool error;

		serializerRef >> id;
		serializerRef >> success;
		serializerRef >> error;

		if (success == true && error == LOGIN_ERROR_NO_ERROR)
		{
			sockRef.m_id = id;
			printf("Login Success\n");
		}
		else
		{
			if (success == false && error == LOGIN_ERROR_WRONG_USERNAME)
			{
				printf("Wrong Username\n\n");
			}
			else if (success == false && error == LOGIN_ERROR_WRONG_PASSWORD)
			{
				printf("Wrong Password\n\n");
			}

			printf("Want to create new account?\n");
			printf("Yes : y, No : n\n\n");

			std::string username;
			std::string password;

			while (true)
			{
					char answer;

					//콘솔 입력이 유니코드(2바이트) 기준으로 변경
					//_getch, _getche는 1바이트씩 처리하기 때문에
					//남아있는 1바이트(null문자가) 버그 발생시킴
					answer = _getwch();

					if (answer == 'Y' || answer == 'y')
					{
						WriteUserInfo(username, password);

						//C2S::Create_Account_Request(sockPtr, username, password);

					}
					else if (answer == 'n' || answer == 'N')
					{
						printf("Write Your ID & Password\n");
						printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

						WriteUserInfo(username, password);

						//C2S::Login_Request(sockPtr, username, password);
					}
					else
					{
						printf("Wrong answer\n");
						continue;
					}

					break;
				
			}
		}
	};

	auto Create_Account_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		bool success;
		serializerRef >> success;

		if (success == true)
		{
			printf("Account creation success\n\n");
		}
		else
		{
			printf("Account creation fail\n\n");
		}

		//C2S::ProcessLogin(sockPtr);
	auto Room_Create_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		bool result;
		unsigned short roomID;

		serializerRef >> result;
		serializerRef >> roomID;

		sockRef.m_roomID = roomID;

		if (result)
		{
			std::cout << roomID << std::endl;
			std::cout << "Chat" << std::endl;
		}
		else
		{
			std::cout << "Fail" << std::endl;
		}
	};

	auto Chat_Print = [](const Socket* const sockPtr) {
	auto Room_Enter_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		bool result;

		serializerRef >> result;

		if (result)
		{
			printf("\nRoom Enter Success\n"); 
			printf("Chat Start\n\n");
			ProcessChat(sockRef);
		}
		else
		{
			printf("Room Enter Fail\n");
			C2S::ProcessRoom(sockRef);
		}
	};

		Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);
	auto Room_Exit_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		bool result;

		serializerRef >> result;

		if (result)
		{
			printf("Room Exit Success\n");
			C2S::ProcessRoom(sockRef);
		}
		else
		{
			printf("Room Exit Fail\n");
		}
	};

	auto Room_Info_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		std::unordered_map<unsigned short, unsigned short> rooms;

		serializerRef >> rooms;

		printf("\n");
		for (auto& room : rooms)
		{
			printf("RoomID : %d", room.first);
			printf("\tUserCount : %d\n", room.second);
		}

		printf("\n1. RoomInfo\n2. EnterRoom\n\n");
	};
}


auto PacketProcess = [](const Socket& sockRef) {

	Serializer serializer(sockRef.m_overlappedStruct.m_buffer);

	Header header;

	serializer.GetHeader(header);

	switch (header.m_type)
	{
	case PacketType::LOGIN_REPLY:
		S2C::Login_Reply(sockRef, serializer);
		break;

	case PacketType::CRERATE_ACCOUNT_REPLY:
		//S2C::Create_Account_Reply(sockRef, serializer);
		break;

	case PacketType::CHAT:

	case PacketType::ROOM_CREATE_REPLY:
		S2C::Room_Create_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_ENTER_REPLY:
		S2C::Room_Enter_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_EXIT_REPLY:
		S2C::Room_Exit_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_INFO_REPLY:
		S2C::Room_Info_Reply(sockRef, serializer);
		break;
	default:
		std::cout << "PacketProcess Error : No such type" << std::endl;
		break;
	}


};