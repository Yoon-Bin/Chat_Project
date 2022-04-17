#pragma once
#include "stdafx.h"
#include "Socket.h"
#include "Room.h"

#include "boost/lockfree/queue.hpp"

#define MAXROOMCOUNT 20

static std::unordered_map<unsigned short, std::shared_ptr<Room>> roomList;
static boost::lockfree::queue<unsigned short> deactivatedRoomList(MAXROOMCOUNT);

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

		std::string message;
		unsigned short userID = sockRef.m_id;

		serializerRef >> message;

		Serializer se;

		se << userID;
		se << message.c_str();

		se.SetHeader(PacketType::CHAT);
		for (auto& user : roomList[sockRef.m_roomID]->m_users)
		{
			if (user->m_id != sockRef.m_id)
			{
				user->OverlapWSAsend(se);
			}
			user->OverlapWSAsend(se);
		};
	};

	auto Create_Account_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		std::string username;
		std::string password;

		serializerRef >> username;
		serializerRef >> password;

		std::string queryString;
		MYSQL_ROW row;

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

		serializer.SetHeader(PacketType::CREATE_ACCOUNT_REPLY);
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

	auto Room_Enter_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		unsigned short roomID;

		
		serializerRef >> roomID;

		Serializer se;

		if (roomList.find(roomID)->second->Enter(sockRef))
		{
			se << true;
		}
		else
		{
			se << false;
		}

		se.SetHeader(PacketType::ROOM_ENTER_REPLY);
		sockRef.OverlapWSAsend(se);
	};

	auto Room_Exit_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		Serializer se;

		if (roomList[sockRef.m_roomID]->Exit(sockRef))
		{
			se << true;
		}
		else
		{
			se << false;
		}

		se.SetHeader(PacketType::ROOM_EXIT_REPLY);
		sockRef.OverlapWSAsend(se);
	};

	auto Room_Info_Reply = [](const Socket& sockRef, Serializer& serializerRef) {

		unsigned short roomPage;

		serializerRef >> roomPage;

		Serializer se;

		std::unordered_map<unsigned short, unsigned short> rooms;

		for (int i = 10 * roomPage + 1; i <= 10 * (roomPage + 1); i++)
		{
			rooms.insert(std::make_pair(i, roomList[i]->m_userCount.load()));
		}

		se << rooms;

		se.SetHeader(PacketType::ROOM_INFO_REPLY);
		sockRef.OverlapWSAsend(se);
			
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

	case PacketType::CREATE_ACCOUNT_REQUEST:
		S2C::Create_Account_Reply(sockRef, serializer);
		break;

	case PacketType::CHAT:
		S2C::Chat_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_ENTER_REQUEST:
		S2C::Room_Enter_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_EXIT_REQUEST:
		S2C::Room_Exit_Reply(sockRef, serializer);
		break;

	case PacketType::ROOM_INFO_REQUEST:
		S2C::Room_Info_Reply(sockRef, serializer);
		break;

	default:
		std::cout << "PacketProcess Error : No such type" << std::endl;
		break;
	}


};

