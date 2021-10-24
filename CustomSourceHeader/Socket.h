#pragma once

#include "SocketUtil.h"

#define MAXBUFFERSIZE 256

enum class SockType
{
	TCP,
	UDP
};

enum class IOType
{
	READ,
	WRITE,
	ACCEPT
};

typedef struct
{
	WSAOVERLAPPED m_wsaOverlapped;
	WSABUF m_wsaBuf;
	unsigned char m_buffer[MAXBUFFERSIZE];
	IOType m_ioType;

} OverlappedStruct, * OverlappedStructPtr;

class Socket
{
public:
	Socket() {};
	Socket(SockType sockType);
	~Socket();

	void SetSockOpt(int level, int optname, int optval);
	void GetSockOpt(int level, int optname);
	void Bind(const EndPoint& endPoint);
	void Listen(int backLog);
	void Connect(const EndPoint& endPoint);

	void Send();
	void Receive();

	bool OverlapAcceptEx(Socket* clientSock);
	bool OverlapConnectEx(Socket& clientSock, EndPoint& endPoint);
	bool OverlapDisconnectEx(Socket& sock);
	int UpdateAcceptContext(Socket& listenSock);
	void OverlapWSAsend(Socket sock);
	void OverlapWSAsend(void* p);
	void OverlapWSArecv();

	void Close();

	SOCKET GetHandle() const;

	char m_receiveBuffer[MAXBUFFERSIZE];

	DWORD m_flag = 0;

	OverlappedStruct m_overlappedStruct;

	LPFN_ACCEPTEX		AcceptEx;
	LPFN_CONNECTEX		ConnectEx;
	LPFN_DISCONNECTEX	DisconnectEx;

	SOCKET m_handle;
private:

	
};

