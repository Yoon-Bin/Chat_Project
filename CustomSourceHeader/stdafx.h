#pragma once

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#define _CRT_SECURE_NO_WARNINGS
//#define WIN32_LEAN_AND_MEAN   

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>
#include <mutex>
#include <memory>
#include <exception>
#include <string>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <stdio.h>
#include <iostream>
#include <bitset>

#include "Exception.h"
#include "EndPoint.h"