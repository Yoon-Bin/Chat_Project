#include "stdafx.h"
#include "CustomHeader.h"

using namespace std;

enum class PacketType
{
	First,
	Second
};


struct Packet
{
	int option;
	int pp;
};

int main()
{
	try
	{
#ifdef _WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw("WSAStartup Fail");
		}
#endif

		char message[1024] = { 0, };
		scanf("%s", &message);

		bool enable = true;

		EndPoint endPoint("192.168.55.52", 4444);

		//(sockaddr*)&endPoint.GetSockAddrIn();

		/*Socket server(SocketType::TCP, true);
		server.Bind(endPoint);*/

		Socket client1(SockType::TCP, true);
		/*Socket client2(SockType::TCP, true);
		Socket client3(SockType::TCP, true);
		Socket client4(SockType::TCP, true);
		Socket client5(SockType::TCP, true);
		Socket client6(SockType::TCP, true);
		Socket client7(SockType::TCP, true);
		Socket client8(SockType::TCP, true);
		Socket client9(SockType::TCP, true);
		Socket client10(SockType::TCP, true);
		Socket client11(SockType::TCP, true);
		Socket client12(SockType::TCP, true);*/
		//client1.m_overlappedStruct.m_ioType = IOType::ACCEPT;

		EndPoint endPoint1("192.168.55.52", 1000);
		/*EndPoint endPoint2("192.168.55.52", 8889);
		EndPoint endPoint3("192.168.55.52", 8890);
		EndPoint endPoint4("192.168.55.52", 8891);
		EndPoint endPoint5("192.168.55.52", 8892);
		EndPoint endPoint6("192.168.55.52", 8893);
		EndPoint endPoint7("192.168.55.52", 8894);
		EndPoint endPoint8("192.168.55.52", 8895);
		EndPoint endPoint9("192.168.55.52", 8896);
		EndPoint endPoint10("192.168.55.52", 8897);
		EndPoint endPoint11("192.168.55.52", 8898);
		EndPoint endPoint12("192.168.55.52", 8899);*/

		client1.Bind(endPoint1);
		/*client2.Bind(endPoint2);
		client3.Bind(endPoint3);
		client4.Bind(endPoint4);
		client5.Bind(endPoint5);
		client6.Bind(endPoint6);
		client7.Bind(endPoint7);
		client8.Bind(endPoint8);
		client9.Bind(endPoint9);
		client10.Bind(endPoint10);
		client11.Bind(endPoint11);
		client12.Bind(endPoint12);*/

		cout << client1.OverlapConnectEx(client1, endPoint) << endl;

		//Sleep(500);
		//cout << client2.OverlapConnectEx(client2, endPoint) << endl;
		////Sleep(500);
		//cout << client3.OverlapConnectEx(client3, endPoint) << endl;
		////Sleep(500);
		//cout << client4.OverlapConnectEx(client4, endPoint) << endl;
		////Sleep(500);
		//cout << client5.OverlapConnectEx(client5, endPoint) << endl;
		////Sleep(500);
		//cout << client6.OverlapConnectEx(client6, endPoint) << endl;
		////Sleep(500); 
		//cout << client7.OverlapConnectEx(client7, endPoint) << endl;
		////Sleep(500);
		//cout << client8.OverlapConnectEx(client8, endPoint) << endl;
		////Sleep(500);
		//cout << client9.OverlapConnectEx(client9, endPoint) << endl;
		////Sleep(500);
		//cout << client10.OverlapConnectEx(client10, endPoint) << endl;
		/*cout << client11.OverlapConnectEx(client11, endPoint) << endl;
		cout << client12.OverlapConnectEx(client12, endPoint) << endl;*/

		////cout << (int)client.m_overlappedStruct.m_ioType << endl;


		Sleep(500);
		char buf[10] = "abc";

		char buf2[10] = "aaaaaaa";

		send(client1.GetHandle(), (const char*)&message, sizeof(message), 0);


		/*send(client7.GetHandle(), (const char*)&buf2, sizeof(buf2), 0);*/

		cout << GetLastErrorAsString() << endl;

		/*char buffer[10] = "abcdefg";

		client.m_overlappedStruct.m_wsaBuf.len = sizeof(buffer);
		client.m_overlappedStruct.m_wsaBuf.buf = (char*)&buffer;*/

		
		recv(client1.GetHandle(), (char*)&client1.m_overlappedStruct.m_buffer, sizeof(client1.m_overlappedStruct.m_buffer), 0);

		printf("%s\n", client1.m_overlappedStruct.m_buffer);

		//client.OverlapWSAsend(client);

		while (true)
		{

		}

	}
	catch (Exception& e)
	{
		printf("%s\n", e.what());
	}

	//std::cout << Text("00000") << std::endl;

	return 0;
}