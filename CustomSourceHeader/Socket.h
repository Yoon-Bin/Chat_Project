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
	char m_buffer[MAXBUFFERSIZE];
	IOType m_ioType;

} OverlappedStruct, *OverlappedStructPtr;

class Socket
{
public:
	Socket() {};
	Socket(SockType sockType);
	~Socket();

	void SetSockOpt(const int level, const int optname, const int optval) const;
	void GetSockOpt(const int level, const int optname) const;
	void Bind(const EndPoint& endPoint);
	void Listen(const int backLog) const;
	void Connect(const EndPoint& endPoint);

	void Send();
	void Receive();

	void OverlapAcceptEx(const Socket* const clientSock) const;
	void UpdateAcceptContext(const Socket* const listenSockPtr) const;
	void OverlapConnectEx(const EndPoint* const endPoint) const;
	bool OverlapDisconnectEx(Socket& sock);
	void OverlapWSAsend(void* const p) const;
	void OverlapWSArecv();

	void Close();

	SOCKET GetHandle() const;

	unsigned short m_id;

	mutable DWORD m_flag = 0;

	mutable OverlappedStruct	m_overlappedStruct;

	mutable LPFN_ACCEPTEX		AcceptEx;
	mutable LPFN_CONNECTEX		ConnectEx;
	mutable LPFN_DISCONNECTEX	DisconnectEx;

	SOCKET m_handle;
private:

	
};

