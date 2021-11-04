#pragma once

#pragma pack(push,1)

#define LOGIN_ERROR_NO_ERROR 0
#define LOGIN_ERROR_WRONG_USERNAME 1
#define LOGIN_ERROR_WRONG_PASSWORD 2

#define MIN_USERINFO_SIZE 8
#define MAX_USERINFO_SIZE 16
#define MAX_USERNAME_SIZE 16
#define MAX_PASSWORD_SIZE 16

enum class PacketType : unsigned char
{
	LOGIN_REQUEST,
	LOGIN_REPLY,
	LOGOUT,
	CRERATE_ACCOUNT_REQUEST,
	CRERATE_ACCOUNT_REPLY,
	CHAT,
	TEST
};

struct Packet_Login_Request
{
	char				size;
	char				type;
	char				username[MAX_USERNAME_SIZE + 1];
	char				password[MAX_PASSWORD_SIZE + 1];
};

struct Packet_Login_Reply
{
	char				size;
	char				type;
	unsigned short		id;
	bool				success;
	char				error;
};

struct Packet_Logout
{
	char				size;
	char				type;
	unsigned short		id;
};

struct Packet_Create_Account_Request
{
	char				size;
	char				type;
	char				username[MAX_USERNAME_SIZE + 1];
	char				password[MAX_PASSWORD_SIZE + 1];
};

struct Packet_Create_Account_Reply
{
	char				size;
	char				type;
	bool				success;

};

#pragma warning(disable : 4200)
struct Packet_Chat
{
	char				size;
	char				type;
	unsigned short		id;
	char				message[];
};

#pragma pack(pop)