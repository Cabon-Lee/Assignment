#pragma once

#include <vector>
#include <list>
#include <map>
#include "Singleton.h"

/// <summary>
/// Object�� ���� �����Ǹ� ������ �÷��� �Ŵ����� ����
/// </summary>
class GarbageCollectionManager : public Singleton<GarbageCollectionManager>
{
public:
	~GarbageCollectionManager();

	void SetRootObject(class Object* pObj);
	void AddObject(class Object* pObj);
	void ClearObject();

	void GarbageCollect(bool isSever);
	

	std::map<unsigned char, std::list<class Object*>>& GetObjectList();
	std::map<unsigned char, std::vector<class Object*>>& GetRootObjectList();

	bool isSeverState = false;
private:
	void ResetFlag(bool isServer);
	void Mark(bool isServer);
	void Sweep(bool isServer);
	void RecursiveMark(class Object* pObj);

	// ���ͺ��ٴ� ����Ʈ�� �����ϴ°� ������ �������� �� �����ϴٰ� ��������
	//std::list<class Object*> serverObjectList;
	//std::list<class Object*> clientObjectList;

	std::vector<class Object*> serverRootObjectList;
	std::vector<class Object*> clientRootObjectList;


	std::list<class Object*>& GetObjectList(unsigned char gid);
	std::vector<class Object*>& GetRootObjectList(unsigned char gid);

	std::map<unsigned char, std::list<class Object*>> threadGroupObjList;
	std::map<unsigned char, std::vector<class Object*>> threadGroupRootObjvec;

	unsigned int stackCount;
	unsigned int delay = 10;
};

