#pragma once
#include "stdafx.h"
#include "Socket.h"


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

		serializer << username;
		serializer << password;
		
		serializer.SetHeader(PacketType::LOGIN_REQUEST);
		sockPtr->OverlapWSAsend(serializer);
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
	};

	auto Chat_Print = [](const Socket* const sockPtr) {

		Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

		printf("%s\n", packet->message);

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

	default:
		std::cout << "PacketProcess Error : No such type" << std::endl;
		break;
	}


};