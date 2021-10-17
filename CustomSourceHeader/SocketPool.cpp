#include "stdafx.h"
#include "SocketPool.h"

SocketPool::SocketPool(int sockCount)
{
	for (int i = 0; i < sockCount; i++)
	{
		std::shared_ptr<Socket> test = std::make_shared<Socket>(SockType::TCP);

		m_sockPool.insert(make_pair(test.get(), test));
	}
}

SocketPool::SocketPool()
{

}

SocketPool::~SocketPool()
{

}

void SocketPool::Insert(Socket* key, std::shared_ptr<Socket> value)
{
	m_sockPool.insert(make_pair(key, value));
}

void SocketPool::Insert(Socket* key)
{
	m_usableSockSet.push(key);
}

void SocketPool::Remove(Socket* key)
{
	m_sockPool.erase(key);
}

void SocketPool::Pop()
{
	m_usableSockSet.pop();
}

size_t SocketPool::GetSize()
{
	return m_sockPool.size();
}

std::shared_ptr<Socket> SocketPool::Find(Socket* key)
{
	return m_sockPool.at(key);
}

Socket* SocketPool::GetUsableSock()
{
	Socket* sock = m_usableSockSet.top();

	return sock;
}