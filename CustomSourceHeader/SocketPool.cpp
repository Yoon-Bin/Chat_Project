#include "stdafx.h"
#include "SocketPool.h"

SocketPool::SocketPool(int sockCount)
{
	for (int i = 0; i < sockCount; i++)
	{
		std::unique_ptr<Socket> sock = std::make_unique<Socket>(SockType::TCP);

		m_fullSockPtrMap.insert(std::make_pair(sock.get(), std::move(sock)));
	}
}

SocketPool::~SocketPool()
{

}

size_t SocketPool::GetFullSockCount() const
{
	return m_fullSockPtrMap.size();
}

size_t SocketPool::GetUsableSockCount() const
{
	return m_fullSockPtrMap.size();
}

void SocketPool::PushUsableSockPtr(Socket* sockPtr)
{
	m_usableSockPtrStack.push(sockPtr);
}

void SocketPool::PopUsableSockPtr()
{
	m_usableSockPtrStack.pop();
}

Socket* SocketPool::GetUsableSockPtr()
{
	Socket* sockPtr = m_usableSockPtrStack.top();

	return sockPtr;
}