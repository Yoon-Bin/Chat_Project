#pragma once

#pragma pack(push,1)

#define MAX_MESSAGE_SIZE 1024

enum PacketType
{
	LOGIN,
	LOGOUT,
	CHAT,
	TEST
};

#pragma warning(disable : 4200)
struct Packet_Chat
{
	char		size;
	char		type;
	int			id;
	char		message[];
};
