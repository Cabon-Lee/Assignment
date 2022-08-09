#include <iostream>
#include "GarbageCollectionManager.h"
#include "ReflectionHelper.h"
#include "OutputMemoryStream.h"
#include "RuntimeClass.h"
#include "ObjectManager.h"
#include "ThreadRegister.h"

GarbageCollectionManager::~GarbageCollectionManager()
{

}

void GarbageCollectionManager::SetRootObject(class Object* pObj)
{
	if (pObj->isSeted) return;
	if (pObj->isRoot || pObj->isReplicated)
	{
		if (isSeverState == IM_SERVER)
		{
			serverRootObjectList.push_back(pObj);
		}
		else
		{
			clientRootObjectList.push_back(pObj);
		}
		pObj->isSeted = true;
	}

	/*
	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	GetRootObjectList(gid).push_back(pObj);

	auto itor = threadGroupRootObjvec.find(gid);
	if (itor != threadGroupRootObjvec.end())
	{
		itor->second.push_back(pObj);
	}
	*/
}

void GarbageCollectionManager::AddObject(class Object* pObj)
{
	std::cout << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;

	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	GetObjectList(gid).push_back(pObj);

	//objectList.push_back(pObj);
}

// Serialize를 위해 사용하지 않는 정보들이 list에 들어가는데, 이를 다시 비우는 용도
void GarbageCollectionManager::ClearObject()
{

}

void GarbageCollectionManager::GarbageCollect(bool isServer)
{
	{
		stackCount = 0;
		ResetFlag(isServer);
		Mark(isServer);
		Sweep(isServer);
	}
}

void GarbageCollectionManager::ResetFlag(bool isServer)
{

	auto objManager = SingleThreadObjectManager::GetInstance();
	
	if (isServer == IM_SERVER)
	{
		auto slist = objManager->serverObjectList;
		for (auto& pObj : slist)
		{
			pObj->GCflag.ResetFlag();
		}
	}
	else
	{
		auto clist = objManager->clientObjectList;
		for (auto& pObj : clist)
		{
			pObj->GCflag.ResetFlag();
		}
	}

	/*
	for (auto& nowObject : objectList)
	{
		nowObject->GCflag.ResetFlag();
	}

	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	auto objectlist = GetObjectList(gid);
	for(auto& pObj : objectlist)
	{
		pObj->GCflag.ResetFlag();
	}
	*/

}

void GarbageCollectionManager::Mark(bool isServer)
{
	if (isServer == IM_SERVER)
	{
		for (auto* pObj : serverRootObjectList)
		{
			RecursiveMark(pObj);
		}
	}
	else
	{
		for (auto& pObj : clientRootObjectList)
		{
			RecursiveMark(pObj);
		}
	}
	

	/*
	std::cout << "마크 중입니다." << std::endl << std::endl;
	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	auto objectlist = GetRootObjectList(gid);
	for (auto& pObj : objectlist)
	{
		RecursiveMark(pObj);
	}

	for (Object* nowObject : rootObjectList)
	{
		std::cout << "마크중 : " << nowObject->GetName() << std::endl;

		RecursiveMark(nowObject);
	}
	*/
}

void GarbageCollectionManager::RecursiveMark(class Object* pObj)
{
	std::cout << "마크 : " << pObj->GetName() << std::endl;
	pObj->GCflag.SetFlag(FLAG_MARKED | FLAG_CHECKED);

	auto typeInfo = STEAM->GetReflectedTypeInfo();
	auto itor = typeInfo.find(pObj->GetName());
	if (itor == typeInfo.end())
	{
		return;
	}

	for (auto& nowMeme : itor->second)
	{
		auto ret = ReflectionHelper::GetInstance()->AcccessToMember(pObj, nowMeme);
		if (ret == nullptr) continue;
		if (ret->myType == EType::PTR)
		{
			Object* tarObj = static_cast<Object*>(ret->data);
			if (tarObj->GCflag.IsDestroy())
			{
				// 더 이상 참조하면 안되는 주소
				ReflectionHelper::GetInstance()->SetMemberPtrToNull(tarObj, nowMeme);
			}
			else
			{
				// 이렇게 처리 하면 살려둔다
				tarObj->GCflag.SetFlag(FLAG_MARKED | FLAG_CHECKED);
			}

			// 이 Object의 멤버 변수 중에 다른 Object가 있는지 검사한다
			RecursiveMark(tarObj);
		}
		else if (ret->myType == EType::VECTOR)
		{
			auto pRet = reinterpret_cast<K_Array<Object*>*>(ret->data);
			if (pRet->IsEmpty() != true)
			{
				auto size = pRet->size();
				Object** obj = pRet->GetData();
				auto name = (*obj)->GetName();
				std::cout << "K_Array " << name << "입니다. 사이즈 : " << size << std::endl;

				for (unsigned int i = 0; i < size; i++)
				{
					RecursiveMark(*(obj + i));
				}
			}
			else
			{
				std::cout << "K_Array "<< nowMeme.GetName() << "는 현재 비어있습니다." << std::endl;
			}
		}
		else if (ret->myType == EType::LIST)
		{
			auto pRet = reinterpret_cast<K_List<Object*>*>(ret->data);
			if (pRet->IsEmpty() != true)
			{
				auto node = pRet->GetHead();
				while (node != nullptr)
				{
					RecursiveMark(node->data);
					node = node->pNext;
				}
			}
		}
	}
}

std::list<class Object*>& GarbageCollectionManager::GetObjectList
(unsigned char gid)
{
	auto itor = threadGroupObjList.find(gid);
	if (itor != threadGroupObjList.end())
	{
		return itor->second;
	}
	std::list<class Object*> null;
	return null;
}



std::vector<class Object*>& GarbageCollectionManager::GetRootObjectList(unsigned char gid)
{
	auto itor = threadGroupRootObjvec.find(gid);
	if (itor != threadGroupRootObjvec.end())
	{
		return itor->second;
	}
	std::vector<class Object*> null;
	return null;
}

std::map<unsigned char, std::list<class Object*>>& GarbageCollectionManager::GetObjectList()
{
	return threadGroupObjList;
}

std::map<unsigned char, std::vector<class Object*>>& GarbageCollectionManager::GetRootObjectList()
{
	return threadGroupRootObjvec;
}

void GarbageCollectionManager::Sweep(bool isServer)
{
	auto objetManager = SingleThreadObjectManager::GetInstance();
	if (isServer == IM_SERVER)
	{
		auto& objs = objetManager->serverObjectList;

		auto it = objs.begin();
		while (it != objs.end())
		{
			if ((*it)->GCflag.IsGarbage())
			{
				std::cout << (*it)->GetName() << " 삭제, ID :" << (*it)->objectID << std::endl;
				auto ptr =(*it);
				delete ptr;
				ptr = nullptr;
				objs.erase(it++);
			}
			else
			{
				it++;
			}
		}
	}
	else
	{
		auto& objs = objetManager->clientObjectList;
		auto it = objs.begin();
		while (it != objs.end())
		{
			if ((*it)->GCflag.IsGarbage())
			{
				std::cout << (*it)->GetName() << " 삭제, ID :" << (*it)->objectID << std::endl;
				auto ptr = (*it);
				delete ptr;
				ptr = nullptr;
				objs.erase(it++);
			}
			else
			{
				it++;
			}
		}
	}

	/*
	auto objs = ObjectManager::GetInstance()->GetObjs();

	for (auto it = objs.begin(); it != objs.end(); it++)
	{
		auto itor = *it;
		if (itor->GCflag.IsGarbage())
		{
			std::cout << itor->GetName() << " 삭제, ID :" << itor->objectID << std::endl;
			delete itor;

			auto target = it++;
			objs.erase(target);
		}
	}
	*/
}
