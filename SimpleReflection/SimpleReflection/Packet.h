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
	std::uintptr_t id;	// 누가 함수를 실행할지
	char type;			// index 혹은 string으로 호출할지
	char funcIndex;
	std::string funcName;
};

const bool GET = true;
const bool PUSH = false;

const bool IM_SERVER = true;
const bool IM_CLIENT = false;

// 패킷 통신 역할을 수행 하는 전역 클래스
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

