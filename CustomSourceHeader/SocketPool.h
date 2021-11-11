#pragma once

#include "Socket.h"

class SocketPool 
{
public:
	SocketPool(int sockCount);
	~SocketPool();

	size_t		GetFullSockCount()		const;
	/*size_t		GetUsableSockCount()	const;

	void		PushUsableSockPtr(Socket* sockPtr);
	void		PopUsableSockPtr();
	Socket*		GetUsableSockPtr();*/

public:
	//�ʿ� ���� ����?
	//std::queue<Socket*> m_usableSockPtrStack;

	std::vector<std::unique_ptr<Socket>> m_fullSockPtrVector;
private:
	
};

