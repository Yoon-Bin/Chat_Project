#pragma once

#include "Socket.h"

#include <unordered_set>

#define MAXUSERCOUNT 4

class Room
{
public:
	Room(unsigned short roomID) : m_roomID(roomID)
	{
	};

	bool Enter(const Socket& sockRef)
	{
		if (m_isFull.load() == true)
		{
			return false;
		}

		if (m_userCount.load() >= MAXUSERCOUNT)
		{
			return false;
		}
		else
		{
			m_userCount.fetch_add(1, std::memory_order_relaxed);
			std::lock_guard<std::mutex> lg(m_mutex);
			m_users.insert(&sockRef);
			sockRef.m_roomID = m_roomID;


			if (m_userCount.load() == MAXUSERCOUNT)
			{
				m_isFull.store(true);
			}

			return true;
		}
	}

	bool Exit(const Socket& sockRef)
	{
		if (m_users.find(&sockRef) == m_users.end())
		{
			return false;
		}
		else
		{
			std::lock_guard<std::mutex> lg(m_mutex);
			if (m_users.erase(&sockRef) == 1)
			{
				m_userCount.fetch_add(-1);

				return true;
			}
			else
			{
				return false;
			}
		}
	}


	std::atomic_bool m_isFull = false;

	std::set<const Socket*> m_users;

	std::atomic_short m_userCount = 0;

private:
	const unsigned short m_roomID;

	std::mutex m_mutex;

};

