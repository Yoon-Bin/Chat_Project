#pragma once

#pragma pack(push,1)

#define MAX_USERNAME_SIZE 16
#define MAX_PASSWORD_SIZE 16

enum PacketType
{
	LOGIN,
	LOGOUT,
	CHAT,
	TEST
};

struct Packet_Login_Request
{
	char		size;
	char		type;
	int			id;
};

struct Packet_Login_Reply
{
	char		size;
	char		type;
	int			id;
	bool		success;
};

struct Packet_Create_Account_Request
{
	char		size;
	char		type;
	char		username[MAX_USERNAME_SIZE];
	char		password[MAX_PASSWORD_SIZE];
};

struct Packet_Create_Account_Reply
{
	char		size;
	char		type;
	bool		success;

};

struct Packet_Logout
{
	char		size;
	char		type;
	int			id;
};

#pragma warning(disable : 4200)
struct Packet_Chat
{
	char		size;
	char		type;
	int			id;
	char		message[];
};
