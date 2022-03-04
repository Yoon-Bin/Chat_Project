#pragma once
#include "stdafx.h"
#include "Socket.h"

auto WriteUserInfo = [](std::string& username, std::string& password) {
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
	auto Login_Request = [](const Socket* const sockPtr, const std::string username, const std::string password) {

		unsigned short packetSize = sizeof(Packet_Login_Request);

		Packet_Login_Request packet;

		strcpy(packet.username, username.c_str());
		strcpy(packet.password, password.c_str());

		sockPtr->OverlapWSAsend(&packet);
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
		
		//C2S::Login_Request(sockPtr, username, password);

		Serializer se;

		std::string message("asdfsdfsdfsdfsfdsfdsfdssdf");

		se << username;
		se << password;
		se << message;
		Header header(se.GetSize(), PacketType::LOGIN_REQUEST);
		se << header;

		sockPtr->OverlapWSAsend(se);
	};
}

namespace S2C
{
	auto Login_Reply = [](const Socket* const sockPtr) {

		Packet_Login_Reply* packet = reinterpret_cast<Packet_Login_Reply*>(sockPtr->m_overlappedStruct.m_buffer);

		if (static_cast<bool>(packet->success) == true && packet->error == LOGIN_ERROR_NO_ERROR)
		{
			sockPtr->m_id = packet->id;
			printf("Login Success\n");

			//loginCV.notify_one();
		}
		else
		{
			if (static_cast<bool>(packet->success) == false && packet->error == LOGIN_ERROR_WRONG_USERNAME)
			{
				printf("Wrong Username\n\n");
			}
			else if (static_cast<bool>(packet->success) == false && packet->error == LOGIN_ERROR_WRONG_PASSWORD)
			{
				printf("Wrong Password\n\n");
			}

			printf("Want to create new account?\n");
			printf("Yes : y, No : n\n\n");

			while (true)
			{
					char answer;

					//콘솔 입력이 유니코드(2바이트) 기준으로 변경
					//_getch, _getche는 1바이트씩 처리하기 때문에
					//남아있는 1바이트(null문자가) 버그 발생시킴
					answer = _getwch();

					if (answer == 'Y' || answer == 'y')
					{
						std::string username;
						std::string password;

						WriteUserInfo(username, password);

						C2S::Create_Account_Request(sockPtr, username, password);

					}
					else if (answer == 'n' || answer == 'N')
					{
						printf("Write Your ID & Password\n");
						printf("Max ID, Password Length is %d\n\n", MAX_USERNAME_SIZE);

						std::string username;
						std::string password;

						WriteUserInfo(username, password);

						C2S::Login_Request(sockPtr, username, password);
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

	auto Create_Account_Reply = [](const Socket* const sockPtr) {

		Packet_Create_Account_Reply* packet = reinterpret_cast<Packet_Create_Account_Reply*>(sockPtr->m_overlappedStruct.m_buffer);

		if (packet->success == true)
		{
			printf("Account creation success\n\n");
		}
		else
		{
			printf("Account creation fail\n\n");
		}

		C2S::ProcessLogin(sockPtr);
	};

	auto Chat_Print = [](const Socket* const sockPtr) {

		Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

		printf("%s\n", packet->message);

	};
}


