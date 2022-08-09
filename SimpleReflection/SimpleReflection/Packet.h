#pragma once
#include "Singleton.h"
#include <queue>
#include <string>
#include <windows.h>
#include <functional>
#include <thread>
#include <map>
#include "ThreadRegister.h"


const char PACKET_CLASS = 9;
const char PACKET_MEMBER = 99;
const char PACKET_RPC = 66;

struct P_ClassInfo
{
	std::string name;
	std::uintptr_t id;
	char flag;
};

struct P_MemberInfo
{
	std::uintptr_t id;
	char index;
	char type;
	std::string data;
};

struct P_PRC
{
	std::uintptr_t id;	// ���� �Լ��� ��������
	char type;			// index Ȥ�� string���� ȣ������
	char funcIndex;
	std::string funcName;
};

const bool GET = true;
const bool PUSH = false;

const bool IM_SERVER = true;
const bool IM_CLIENT = false;

// ��Ŷ ��� ������ ���� �ϴ� ���� Ŭ����
class Packet : public Singleton<Packet>
{
public:
	Packet() 
	{ 
		::InitializeCriticalSection(&g_cs); 
	}
	~Packet() { ::DeleteCriticalSection(&g_cs); }

	void AccessToServer(std::queue<char>& que, char& packetType, bool isGet);
	void AccessToClient(std::queue<char>& que, char& packetType, bool isGet);

	void CanclationSever(size_t size);
	void CanclationClinet(size_t size);

	std::queue<char> toServer;
	std::queue<char> toClient;
	CRITICAL_SECTION g_cs;

	unsigned char groupID = 0;

	std::map<std::thread::id, ThreadID> theadID;
};

