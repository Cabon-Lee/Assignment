#pragma once

#include <vector>
#include <list>
#include <map>
#include "Singleton.h"

/// <summary>
/// Object가 동적 생성되면 가비지 컬렉션 매니저에 들어간다
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

	// 벡터보다는 리스트로 관리하는게 삭제와 생성에서 더 용이하다고 생각했음
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

