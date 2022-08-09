#pragma once
#include "Singleton.h"
#include <thread>
#include <map>

#define TID std::this_thread::get_id()

#define TIDCHECK(itor) auto& threadID = Packet::GetInstance()->theadID;  \
auto itor = threadID.find(std::this_thread::get_id()); \
if(itor != threadID.end()) \

#define GETTLIST auto itorator = threadObjectList.find(TID); \
if(itorator != threadObjectList.end()) \
	itorator->second \

#define GETTLISTOBJ(list) std::list<class Object*> list; \
auto itorator = threadObjectList.find(TID); \
if(itorator != threadObjectList.end()) \
	list = itorator->second;

using GID = unsigned char;

// 기본적으로 thread::id만으로 식별이 충분히 가능하지만,
// 서버나 클라이언트에서 다른 thread를 운용하는 경우, 문제가 생길 수 있다
// 따라서 서로 다른 thread라고 하더라도 같은 group으로 묶여있다면 같은 서버 같은 클라도 구분 지을 수 있다
struct ThreadID
{
	bool isServer;
	unsigned char threadGroup;
};

class ThreadRegister : public Singleton<ThreadRegister>
{
public:
	ThreadRegister();
	~ThreadRegister();

	void ThreadRegist(unsigned char grounpID, std::thread::id id, bool isSever);
	bool IsThisServer(std::thread::id id);
	GID GetGID(std::thread::id id);

private:
	std::map<std::thread::id, GID> groupThreadID;
};


