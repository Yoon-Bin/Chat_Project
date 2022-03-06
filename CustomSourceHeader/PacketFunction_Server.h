#pragma once
#include "stdafx.h"
#include "Socket.h"

auto ExecuteQuery = [](const char* queryString, MYSQL_ROW& row)->bool {

	MYSQL conn;
	MYSQL* connPtr = nullptr;
	MYSQL_RES* result;
	int stat;

	mysql_init(&conn);

	connPtr = mysql_real_connect(&conn, "127.0.0.1", "root", "tjqjvmfajyb12#", "sampledatabase", 3306, (char*)NULL, 0);

	if (connPtr == NULL)
	{
		fprintf(stderr, "Mysql Connection Error : %s\n", mysql_error(&conn));

		return false;
	}

	stat = mysql_query(&conn, queryString);

	if (stat != 0)
	{
		fprintf(stderr, "Mysql Err : %s \n", mysql_error(&conn));

		return false;
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

	return true;
};


namespace S2C
{
	auto Chat_Reply = [](const Socket& sockRef, Serializer& serializerRef) {


		//Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

		//sockPtr->OverlapWSAsend(packet);
	};

	auto Create_Account_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		std::string username;
		std::string password;

		serializerRef >> username;
		serializerRef >> password;

		std::string queryString;
		MYSQL_ROW row;

		//파라미터 팩 사용으로 querymaker 구현
		queryString.append("insert into user(username,password) values('");
		queryString.append(username);
		queryString.append("', '");
		queryString.append(password);
		queryString.append("')");

		bool result = ExecuteQuery(queryString.c_str(), row);

		Serializer serializer;

		if (result)
		{
			serializer << true;
		}
		else
		{
			serializer << false;
		}

		serializer.SetHeader(PacketType::CRERATE_ACCOUNT_REPLY);
		sockRef.OverlapWSAsend(serializer);
	};

	auto Login_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		std::string username;
		std::string password;

		serializerRef >> username;
		serializerRef >> password;

		MYSQL_ROW row;
		std::string queryString;

		queryString.append("select password from user where username='");
		queryString.append(username.c_str());
		queryString.append("'");

		bool result = ExecuteQuery(queryString.c_str(), row);

		Serializer serializer;

		if (result)
		{
			//해당 username row 없음
			if (row == nullptr)
			{
				serializer << static_cast<unsigned short>(0);
				serializer << false;
				serializer << static_cast<unsigned char>(LOGIN_ERROR_WRONG_USERNAME);
			}
			//해당 username row 있음
			else
			{
				serializer << static_cast<unsigned short>(sockRef.m_handle);

				//password 일치
				if (strcmp(row[0], password.c_str()) == 0)
				{
					serializer << true;
					serializer << static_cast<unsigned char>(LOGIN_ERROR_NO_ERROR);
				}
				//password 불일치
				else
				{
					serializer << false;
					serializer << static_cast<unsigned char>(LOGIN_ERROR_WRONG_PASSWORD);
				}
			}
		}
		else
		{
			serializer << static_cast<unsigned short>(0);
			serializer << false;
			serializer << static_cast<unsigned char>(DB_CONNECTION_FAIL);
		}

		serializer.SetHeader(PacketType::LOGIN_REPLY);
		sockRef.OverlapWSAsend(serializer);
	};
}

auto PacketProcess = [](const Socket& sockRef) {

	Serializer serializer(sockRef.m_overlappedStruct.m_buffer);

	Header header;

	serializer.GetHeader(header);

	switch (header.m_type)
	{
	case PacketType::LOGIN_REQUEST:
		S2C::Login_Reply(sockRef, serializer);
		break;

	case PacketType::CRERATE_ACCOUNT_REQUEST:
		S2C::Create_Account_Reply(sockRef, serializer);
		break;

	case PacketType::CHAT:

	default:
		std::cout << "PacketProcess Error : No such type" << std::endl;
		break;
	}


};

