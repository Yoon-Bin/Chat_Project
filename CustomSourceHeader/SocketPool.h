#pragma once

#include "Socket.h"

class SocketPool 
{
public:
	SocketPool(int sockCount, bool isBinded, SockType protocol);
	~SocketPool();

	size_t GetFullSockCount() const;

	std::vector<std::unique_ptr<Socket>> m_fullSockPtrVector;
};

