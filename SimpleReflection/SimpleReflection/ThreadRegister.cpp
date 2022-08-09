#include "ThreadRegister.h"
#include "Packet.h"
#include "ObjectManager.h"
#include "GarbageCollectionManager.h"

ThreadRegister::ThreadRegister()
{

}

ThreadRegister::~ThreadRegister()
{

}


void ThreadRegister::ThreadRegist(unsigned char gid, std::thread::id id, bool isSever)
{
	ThreadID groupId;
	groupId.isServer = isSever;
	groupId.threadGroup = gid;

	auto itor = groupThreadID.find(id);
	if (itor == groupThreadID.end())
	{
		// 현재 자료구조에 들어있지 않을때만
		groupThreadID.insert({ id, gid });
	}
	
	Packet::GetInstance()->theadID.insert({ id, groupId });
	ObjectManager::GetInstance()->GetThreadGroupObjectList().insert({gid, std::list<class Object*> ()});
	auto gc = GarbageCollectionManager::GetInstance();
	gc->GetObjectList().insert({ gid, std::list<class Object*>() });
	gc->GetRootObjectList().insert({ gid, std::vector<class Object*>() });
}

bool ThreadRegister::IsThisServer(std::thread::id id)
{
	auto threadID = Packet::GetInstance()->theadID;
	auto itor = threadID.find(id);
	if (itor != threadID.end())
	{
		return itor->second.isServer;
	}
}

GID ThreadRegister::GetGID(std::thread::id id)
{
	auto itor = groupThreadID.find(id);
	if (itor != groupThreadID.end())
	{
		return itor->second;
	}
}
