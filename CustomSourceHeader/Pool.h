#pragma once

using namespace std;

template <class T>
class Pool
{
public:
	Pool();
	~Pool();
	
	void Insert(SOCKET key, shared_ptr<T> element);
	void Remove(SOCKET key);

	int GetSize() const;
	shared_ptr<T> Find(SOCKET key) const;

	unordered_map<SOCKET, shared_ptr<T>> m_pool;
};

template <class T>
Pool<T>::Pool()
{
	//m_pool.find()
}

template <class T>
Pool<T>::~Pool()
{
	
}

template <class T>
void Pool<T>::Insert(SOCKET key, shared_ptr<T> element)
{
	m_pool.insert(make_pair(key, element));
}

template <class T>
void Pool<T>::Remove(SOCKET key)
{
	m_pool.erase(key);
}

template <class T>
int Pool<T>::GetSize() const
{
	return m_pool.size();
}

template <class T>
shared_ptr<T> Pool<T>::Find(SOCKET key) const
{
	return m_pool.at(key);
}