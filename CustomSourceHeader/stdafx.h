﻿#pragma once

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#pragma comment(lib, "libmysql.lib")
#define _CRT_SECURE_NO_WARNINGS
//#define WIN32_LEAN_AND_MEAN   

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

//#include <boost/thread.hpp>
//#include <boost/asio/thread_pool.hpp>

#include <mysql.h>

#include <thread>
#include <mutex>
#include <memory>
#include <exception>
#include <string>
#include <sstream>
#include <stack>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>
#include <bitset>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>

#include "Exception.h"
#include "EndPoint.h"
#include "Packet.h"
#include "Serializer.h"