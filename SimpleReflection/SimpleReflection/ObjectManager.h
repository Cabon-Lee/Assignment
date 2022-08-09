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
/// Root ������Ʈ���� �������ִ� �Ŵ����� Ŭ����
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


	// ������Ʈ �Ŵ����� �������� ����,
	// ������Ʈ ���� �� LocalReplication�� ������ �о�־� �� ƴ�� ����
	// �Ұ����ϰ� �̷��� ����մϴ�
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

// �̱� ������ �������� Ȱ���ϴ� ������Ʈ �Ŵ����Դϴ�.
// ������ Ŭ���� ���ÿ��� ���ư���, �ڷᱸ���� ������ �����մϴ�.
// �� ������Ʈ�� �̱������� ����ϴ� ������, Ŭ���� ���� �� ������ �۾����� AddObject�� ȣ���ϱ� �����Դϴ�.
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


