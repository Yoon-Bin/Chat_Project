#pragma once

#define DB_CONNECTION_FAIL 255
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
private:
	char				size = static_cast<char>(sizeof(Packet_Login_Request));;
	char				type = static_cast<char>(PacketType::LOGIN_REQUEST);;

public:
	char				username[MAX_USERNAME_SIZE + 1] = { 0, };
	char				password[MAX_PASSWORD_SIZE + 1] = { 0, };
};

struct Packet_Login_Reply
{
private:
	char				size = static_cast<char>(sizeof(Packet_Login_Reply));;
	char				type = static_cast<char>(PacketType::LOGIN_REPLY);

public:
	unsigned short		id;
	bool				success;
	char				error;
};

struct Packet_Logout
{
private:
	char				size = static_cast<char>(sizeof(Packet_Logout));
	char				type = static_cast<char>(PacketType::LOGOUT);

public:
	unsigned short		id;
};

struct Packet_Create_Account_Request
{
private:
	char				size = static_cast<char>(sizeof(Packet_Create_Account_Request));
	char				type = static_cast<char>(PacketType::CRERATE_ACCOUNT_REQUEST);

public:
	char				username[MAX_USERNAME_SIZE + 1] = { 0, };
	char				password[MAX_PASSWORD_SIZE + 1] = { 0, };
};

struct Packet_Create_Account_Reply
{
private:
	char				size = static_cast<char>(sizeof(Packet_Create_Account_Reply));
	char				type = static_cast<char>(PacketType::LOGIN_REPLY);

public:
	bool				success;

};


#pragma warning(disable : 4200)
struct Packet_Chat
{
public:
	char				size;
	char				type;
	unsigned short		id;
	char				message[];
};

