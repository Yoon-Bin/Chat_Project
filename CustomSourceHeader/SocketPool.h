#pragma once

#include "Socket.h"

class SocketPool 
{
public:
	SocketPool(int sockCount);
	~SocketPool();

	size_t		GetFullSockCount()		const;
	size_t		GetUsableSockCount()	const;

	void		PushUsableSockPtr(Socket* sockPtr);
	void		PopUsableSockPtr();
	Socket*		GetUsableSockPtr();

public:
	std::stack<Socket*> m_usableSockPtrStack;

	std::unordered_map<Socket*, std::unique_ptr<Socket>> m_fullSockPtrMap;
private:
	
};

