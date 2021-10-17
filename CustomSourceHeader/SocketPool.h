#pragma once

#include "Socket.h"

// 家南 钱 积己 棺 贸府 苞沥
// 家南 固府 积己 -> AcceptEx -> Iocp 殿废
// -> UpdateAcceptContext


class SocketPool 
{
public:
	SocketPool(int sockCount);
	SocketPool();
	~SocketPool();

	void Insert(Socket* key, std::shared_ptr<Socket> value); ///<犁荤侩 且 荐 乐绰 家南(傈眉 家南 格废) Insert
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

