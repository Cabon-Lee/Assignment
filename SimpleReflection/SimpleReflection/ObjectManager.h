#pragma once
#include "Singleton.h"
#include <list>
#include <vector>
#include <queue>
#include <thread>
#include <map>
#include "PacketProcessor.h"
#include "ThreadRegister.h"


/// <summary>
/// Root 오브젝트들을 관리해주는 매니저급 클래스
/// </summary>

class ObjectManager : public Singleton<ObjectManager>
{
public:
	ObjectManager();
	~ObjectManager();
	void AddObject(class Object* pObj);
	void Start();
	void Tick();
	void Tick(class LocalReplication* pLocal);


	std::list<class Object*>& GetObjs() { return objectList; }
	std::map<GID, std::list<class Object*>>& GetThreadGroupObjectList() { return threadGroupObjectList; };
	std::list<class Object*>& GetGIDObjects(GID gid);

	void ClearObjects();
	void IsLocalMemory(bool val) { isLocalMemory = val; }

	void CreateObjectWithPacket(GID gid, PACKET& packet, class LocalReplication* lopl);


	std::list<class Object*> serverList;
	std::list<class Object*> clientList;


	// 오브젝트 매니저가 전역으로 돌고,
	// 오브젝트 생성 시 LocalReplication의 정보를 밀어넣어 줄 틈이 없어
	// 불가피하게 이렇게 사용합니다
	class LocalReplication* pServerLocal;
	bool isServer = false;
private:
	std::list<class Object*> objectList;
	std::queue<class Object*> newObjectQue;



	std::map<GID, std::list<class Object*>> threadGroupObjectList;
	std::list<class Object*>& GetObjectList(GID gid);

	bool isLocalMemory;
	void AddObjectToList(std::thread::id id, class Object* pObj);
	void AddObjectToList(class Object* pObj);
	void AddObjectToList(GID gid, class Object* pObj);
	void AddObjectOnServer(class Object* pObj);
	void AddObjectOnClinet(class Object* pObj);

	CRITICAL_SECTION cs;
};

// 싱글 스레드 버전에서 활용하는 오브젝트 매니저입니다.
// 서버와 클라의 로컬에서 돌아가며, 자료구조를 별도로 관리합니다.
// 이 오브젝트를 싱글톤으로 사용하는 이유는, 클래스 생성 시 별도의 작업없이 AddObject를 호출하기 위함입니다.
class SingleThreadObjectManager : public Singleton<SingleThreadObjectManager>
{
public:
	SingleThreadObjectManager();
	~SingleThreadObjectManager();
	void AddObject(class Object* pObj);
	void Start();
	void Tick();

	void IsLocalMemory(bool val) { isLocalMemory = val; }
	void CreateObjectWithPacket(PACKET& packet);

	bool isSeverState = false;
	std::list<class Object*> serverObjectList;
	std::list<class Object*> clientObjectList;

	class LocalReplication* serverReplication;
	class LocalReplication* clientReplication;

	std::queue<class Object*> serverNewObjectQue;
	std::queue<class Object*> clientNewObjectQue;

	std::uintptr_t prevPtr = 0;
	size_t prevPacketSize = 0;
	
private:
	void AddObjectToList(class Object* pObj);

	bool isLocalMemory;
	void AddRepliObjectOnServer(class Object* pObj);
	void AddRepliObjectOnClinet(class Object* pObj);
	void AddObjectOnServer(class Object* pObj);
	void AddObjectOnClinet(class Object* pObj);
	
};


