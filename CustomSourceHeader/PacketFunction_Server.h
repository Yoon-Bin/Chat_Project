#pragma once
#include "stdafx.h"
#include "Socket.h"

auto ExecuteQuery = [](const std::string queryString, MYSQL_ROW& row)->bool {

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

	stat = mysql_query(&conn, queryString.c_str());

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
	auto Chat_Reply = [](const Socket* const sockPtr) {

		Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

		sockPtr->OverlapWSAsend(packet);
	};

	auto Create_Account_Reply = [](const Socket* const sockPtr) {

		Packet_Create_Account_Request* packet = reinterpret_cast<Packet_Create_Account_Request*>(sockPtr->m_overlappedStruct.m_buffer);

		Packet_Create_Account_Reply replyPacket;

		std::string queryString;
		MYSQL_ROW row;

		queryString.append("insert into user(username,password) values('");
		queryString.append(packet->username);
		queryString.append("', '");
		queryString.append(packet->password);
		queryString.append("')");

		bool result = ExecuteQuery(queryString, row);

		if (result)
		{
			replyPacket.success = true;
		}
		else
		{
			replyPacket.success = false;
		}

		sockPtr->OverlapWSAsend(&replyPacket);
	};

	auto Login_Reply = [](const Socket* const sockPtr) {

		Packet_Login_Request* packet = reinterpret_cast<Packet_Login_Request*>(sockPtr->m_overlappedStruct.m_buffer);

		MYSQL_ROW row;
		std::string queryString;

		queryString.append("select password from user where username='");
		queryString.append(packet->username);
		queryString.append("'");

		bool result = ExecuteQuery(queryString, row);

		Packet_Login_Reply replyPacket;

		if (result)
		{
			//해당 username row 없음
			if (row == nullptr)
			{
				replyPacket.id = 0;
				replyPacket.success = false;
				replyPacket.error = LOGIN_ERROR_WRONG_USERNAME;
			}
			//해당 username row 있음
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
		}
		else
		{
			replyPacket.id = 0;
			replyPacket.success = false;
			replyPacket.error = DB_CONNECTION_FAIL;
		}
	};
}

namespace C2S
{
	auto Chat_Reply = [](const Socket* const sockPtr) {

		Packet_Chat* packet = reinterpret_cast<Packet_Chat*>(sockPtr->m_overlappedStruct.m_buffer);

		//printf("%d : %s\n", packet->id, packet->message);
		printf("%d : %s\n", (int)sockPtr->m_handle, packet->message);
	};
}












