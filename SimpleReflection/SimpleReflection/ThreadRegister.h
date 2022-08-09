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

// �⺻������ thread::id������ �ĺ��� ����� ����������,
// ������ Ŭ���̾�Ʈ���� �ٸ� thread�� ����ϴ� ���, ������ ���� �� �ִ�
// ���� ���� �ٸ� thread��� �ϴ��� ���� group���� �����ִٸ� ���� ���� ���� Ŭ�� ���� ���� �� �ִ�
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


