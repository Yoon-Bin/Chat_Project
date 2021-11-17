#pragma once
#include "stdafx.h"
#include "Socket.h"

auto WriteUserInfo = [](std::string& info, const char* infoType) {
	while (true)
	{
		printf("%s : ", infoType);
		std::getline(std::cin, info);

		if (info.length() > MAX_USERINFO_SIZE) {
			printf("Too Long, Write Again\n\n");
			continue;
		}
		else
			break;
	}
};

auto CreateAccount = [](Socket* sockPtr) {

	printf("Want to create new account?\n");
	printf("Yes : y, No : n\n");

	while (true)
	{
		char answer[1];
		scanf("%c", answer);
		getchar();

		if ((int)answer[0] == (int)'y' || (int)answer[0] == (int)'Y')
		{
			std::string username;
			std::string password;

			WriteUserInfo(username, "Username");
			WriteUserInfo(password, "Password");

			Packet_Create_Account_Request packet;

			packet.size = static_cast<char>(sizeof(Packet_Create_Account_Request));
			packet.type = static_cast<char>(PacketType::CRERATE_ACCOUNT_REQUEST);
			strcpy(packet.username, username.c_str());
			strcpy(packet.password, password.c_str());

			sockPtr->OverlapWSAsend(&packet);

			break;
		}
		else if ((int)answer[0] == (int)'n' || (int)answer[0] == (int)'N')
		{

			break;
		}
		else
		{
			printf("Wrong answer\n");
			break;
		}
	}
};

auto ExecuteQuery = [](std::string queryString, MYSQL_ROW& row)->int {

	MYSQL conn;
	MYSQL* connPtr = nullptr;
	MYSQL_RES* result;
	int stat;

	mysql_init(&conn);

	connPtr = mysql_real_connect(&conn, "127.0.0.1", "root", "tjqjvmfajyb12#", "sampledatabase", 3306, (char*)NULL, 0);

	if (connPtr == NULL)
	{
		fprintf(stderr, "Mysql Connection Error : %s\n", mysql_error(&conn));

		return 1;
	}

	stat = mysql_query(&conn, queryString.c_str());

	if (stat != 0)
	{
		fprintf(stderr, "Mysql Err : %s \n", mysql_error(&conn));
		
		return 1;
	}

	result = mysql_store_result(connPtr);

	if (result != nullptr)
	{
		row = mysql_fetch_row(result);
	}
	else
	{
		row = nullptr;
	}

	mysql_close(connPtr);

	return 0;
};

auto PacketProcess_Chat_Resend = [](Socket* sockPtr) {

	Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

	//printf("%d : %s\n", packet->id, packet->message);
	printf("%d : %s\n", (int)sockPtr->m_handle, packet->message);

	sockPtr->OverlapWSAsend(packet);
};

auto PacketProcess_Chat_Print = [](Socket* sockPtr) {

	Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);
	
	//printf("%d : %s\n", packet->id, packet->message);
	printf("%d : %s\n", (int)sockPtr->m_handle, packet->message);
};

auto PacketProcess_Login_Request = [](Socket* sockPtr) {

	Packet_Login_Request* packet = reinterpret_cast<Packet_Login_Request*>(sockPtr->m_overlappedStruct.m_buffer);

	MYSQL_ROW row;
	std::string queryString;

	queryString.append("select password from user where username='");
	queryString.append(packet->username);
	queryString.append("'");

	ExecuteQuery(queryString, row);

	Packet_Login_Reply replyPacket;

	replyPacket.size = static_cast<char>(sizeof(Packet_Login_Reply));
	replyPacket.type = static_cast<char>(PacketType::LOGIN_REPLY);

	//해당 username row 없음
	if (row == nullptr)
	{
		replyPacket.id = 0;
		replyPacket.success = false;
		replyPacket.error = LOGIN_ERROR_WRONG_USERNAME;
	}
	else
	{
		replyPacket.id = static_cast<unsigned short>(sockPtr->m_handle);

		//password 일치
		if (strcmp(row[0], packet->password) == 0)
		{
			replyPacket.success = true;
			replyPacket.error = LOGIN_ERROR_NO_ERROR;
		}
		//password 불일치
		else
		{
			replyPacket.success = false;
			replyPacket.error = LOGIN_ERROR_WRONG_PASSWORD;
		}
	}
	sockPtr->OverlapWSAsend(&replyPacket);

};

auto PacketProcess_Create_Account_Request = [](Socket* sockPtr) {

	Packet_Create_Account_Request* packet = reinterpret_cast<Packet_Create_Account_Request*>(sockPtr->m_overlappedStruct.m_buffer);

	Packet_Create_Account_Reply replyPacket;

	replyPacket.size = static_cast<char>(sizeof(Packet_Create_Account_Reply));
	replyPacket.type = static_cast<char>(PacketType::CRERATE_ACCOUNT_REPLY);

	std::string queryString;
	MYSQL_ROW row;

	queryString.append("insert into user(username,password) values('");
	queryString.append(packet->username);
	queryString.append("', '");
	queryString.append(packet->password);
	queryString.append("')");

	int result = ExecuteQuery(queryString, row);

	if (result == 0)
	{
		replyPacket.success = true;
	}
	else
	{
		replyPacket.success = false;
	}

	sockPtr->OverlapWSAsend(&replyPacket);
};

auto PacketProcess_Login_Reply = [](Socket* sockPtr) {

	Packet_Login_Reply* packet = reinterpret_cast<Packet_Login_Reply*>(sockPtr->m_overlappedStruct.m_buffer);

	if (static_cast<bool>(packet->success) == true && packet->error == LOGIN_ERROR_NO_ERROR)
	{
		sockPtr->m_id = packet->id;
		printf("Login Success\n");
	}
	else if (static_cast<bool>(packet->success) == false && packet->error == LOGIN_ERROR_WRONG_USERNAME)
	{
		printf("Wrong Username\n");
		CreateAccount(sockPtr);
	}
	else if (static_cast<bool>(packet->success) == false && packet->error == LOGIN_ERROR_WRONG_PASSWORD)
	{
		printf("Wrong Password\n");
		CreateAccount(sockPtr);
	}
};

auto PacketProcess_Create_Account_Reply = [](Socket* sockPtr) {

	Packet_Create_Account_Reply* packet = reinterpret_cast<Packet_Create_Account_Reply*>(sockPtr->m_overlappedStruct.m_buffer);

	if (packet->success == true)
	{
		printf("Account creation success\n");
	}
	else
	{
		printf("Account creation fail\n");
	}
};