#pragma once

#include "Socket.h"

// ���� Ǯ ���� �� ó�� ����
// ���� �̸� ���� -> AcceptEx -> Iocp ���
// -> UpdateAcceptContext


class SocketPool 
{
public:
	SocketPool(int sockCount);
	SocketPool();
	~SocketPool();

	void Insert(Socket* key, std::shared_ptr<Socket> value); ///<���� �� �� �ִ� ����(��ü ���� ���) Insert
	void Insert(Socket* key);
	void Remove(Socket* key);
	void Pop();

	std::shared_ptr<Socket> Find(Socket* key);

	size_t GetSize();

	Socket* GetUsableSock();

public:
	std::stack<Socket*> m_usableSockSet;

	std::unordered_map<Socket*, std::shared_ptr<Socket>> m_sockPool;
private:
	
};

