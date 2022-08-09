#pragma once
#include "SimpleGame.h"
#include "Headers.h"
#include "Singleton.h"
#include "OutputMemoryStream.h"
#include "GarbageCollectionManager.h"

#include "Returner.h"

#define STEAM OutputMemoryStream::GetInstance()

#define PUTIN_CLS(className) className cls##className; \
Serialize<className>(*STEAM, cls##className.GetDataType(), &cls##className); \

class ReflectionHelperControl
{
public:
	ReflectionHelperControl() 
	{ 
		ObjectManager::GetInstance()->IsLocalMemory(true);
		SingleThreadObjectManager::GetInstance()->IsLocalMemory(true);
		std::cout << "Reflection�� ���� Class Serialize ����" << std::endl; 
	}
	~ReflectionHelperControl()
	{
		ObjectManager::GetInstance()->IsLocalMemory(false);
		SingleThreadObjectManager::GetInstance()->IsLocalMemory(false);
		FunctionReplication::GetInstance()->ClearObject();
		std::cout << "Reflection�� ���� Class Serialize ����" << std::endl << std::endl;
	};
};

#define RELECTION_START() ReflectionHelperControl re;



class ReflectionHelper : public Singleton<ReflectionHelper>
{
public:
	ReflectionHelper()
	{
		
		RELECTION_START()
			// ���� ������ �ϳ� ����� ������ �� �ø�������� ������ ������ �ϰ� �Ѵ�
			// �ø���������� ������ ������ ����, ��� ���� Ŭ������ ���� ������ �ȳ����� ��� �̷������
			PUTIN_CLS(Custom)
			PUTIN_CLS(Actor)
			PUTIN_CLS(Actress)
			PUTIN_CLS(Component)
			PUTIN_CLS(Item)
			PUTIN_CLS(World)
			PUTIN_CLS(Inventory)
			PUTIN_CLS(Pawn)
			PUTIN_CLS(Hero)
			PUTIN_CLS(Monster)
			PUTIN_CLS(CharacterStatus)
	}

	static Returner* GetReturnByType(Object* scr, const MemberVariable& mem)
	{
		switch (mem.GetType())
		{
		case EType::STRING:
		{
			// ���� ��Ʈ���� �����ؼ� ���ڸ� �߰��ϰ��� �Ѵٸ�, memsize�� �����ͼ� �� ũ�⸦ Ű���ִ��� �ؾ��ҵ�?
			auto* string = reinterpret_cast<std::string*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnValue<std::string>(string));
			ret->myType = EType::STRING;
			return ret;
		}
		case EType::INT:
		{
			int* integer = reinterpret_cast<int*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnValue<int>(integer));
			ret->myType = EType::INT;
			return ret;
		}
		case EType::FLOAT:
		{
			float* floatPoint = reinterpret_cast<float*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnValue<float>(floatPoint));
			ret->myType = EType::FLOAT;
			return ret;
		}
		case EType::PTR:
		{
			void* ptr = *reinterpret_cast<void**>((std::byte*)scr + mem.GetOffset());
			// nullptr�� �˻��ϰų�, �ʱ�ȭ������ �ʾ����� ���� ���Ѵ�
			if (ptr == nullptr || (unsigned long long)ptr == 0xcdcdcdcdcdcdcdcd)
				return nullptr;
			auto ret = static_cast<Returner*>(new ReturnValue<Object>((Object*)ptr));
			ret->myType = EType::PTR;
			return ret;
		}
		case EType::VECTOR:
		{
			size_t arrSize = 0;
			// data*�� ����Ű�� �迭�� 0���� ���Ұ� pObj�� �ȴ�
			//Object* pObj = ReadArray<Object>((std::byte*)scr + mem.GetOffset());

			Object** pObj = *reinterpret_cast<Object***>((std::byte*)scr + mem.GetOffset());
			if (pObj == nullptr || (unsigned long long)pObj == 0xcdcdcdcdcdcdcdcd)
			{
				pObj = nullptr;
				arrSize = 0;
			}
			else
			{
				arrSize = ReadSize<Object*>((std::byte*)scr + mem.GetOffset());
			}
			
			K_Array<Object*>* pArr = new K_Array<Object*>(pObj, arrSize);
			auto ret = static_cast<Returner*>(new ReturnValue<K_Array<Object*>>(pArr));
			ret->myType = EType::VECTOR;
			return ret;
			
		}
		case EType::LIST:
		{
			auto pHeadPrev = reinterpret_cast<K_Node<Object*>*>((std::byte*)scr + mem.GetOffset());
			if (pHeadPrev != nullptr)
			{
				auto pHead = pHeadPrev->pNext;
				if (pHead != nullptr)
				{
					auto pObj = pHead->data;
					K_List<Object*>* pList = new K_List<Object*>(pHead);
					auto ret = static_cast<Returner*>(new ReturnValue<K_List<Object*>>(pList));
					ret->myType = EType::LIST;
					return ret;
				}
			}
			return nullptr;
		}
		return nullptr;
		}
	}

	static Returner* GetReturnByString(Object* scr, const MemberVariable& mem)
	{
		switch (mem.GetType())
		{
		case EType::STRING:
		{
			auto* string = reinterpret_cast<std::string*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnString(*string));
			ret->myType = EType::STRING;
			return ret;
		}
		case EType::INT:
		{
			int* integer = reinterpret_cast<int*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnString(std::to_string(*integer)));
			ret->myType = EType::INT;
			return ret;
		}
		case EType::FLOAT:
		{
			float* floatPoint = reinterpret_cast<float*>((std::byte*)scr + mem.GetOffset());
			auto ret = static_cast<Returner*>(new ReturnString(std::to_string(*floatPoint)));
			ret->myType = EType::FLOAT;
			return ret;
		}
		case EType::PTR:
		{
			void* ptr = *reinterpret_cast<void**>((std::byte*)scr + mem.GetOffset());
			// nullptr�� �˻��ϰų�, �ʱ�ȭ������ �ʾ����� ���� ���Ѵ�
			if (ptr == nullptr || (unsigned long long)ptr == 0xcdcdcdcdcdcdcdcd)
				return nullptr;
			auto ret = static_cast<Returner*>(new ReturnValue<Object>((Object*)ptr));
			ret->myType = EType::PTR;
			return ret;
		}
		case EType::VECTOR:
		{
			size_t arrSize = 0;
			// data*�� ����Ű�� �迭�� 0���� ���Ұ� pObj�� �ȴ�
			//Object* pObj = ReadArray<Object>((std::byte*)scr + mem.GetOffset());

			Object** pObj = *reinterpret_cast<Object***>((std::byte*)scr + mem.GetOffset());
			if (pObj == nullptr || (unsigned long long)pObj == 0xcdcdcdcdcdcdcdcd)
			{
				pObj = nullptr;
				arrSize = 0;
			}
			else
			{
				arrSize = ReadSize<Object*>((std::byte*)scr + mem.GetOffset());
			}

			K_Array<Object*>* pArr = new K_Array<Object*>(pObj, arrSize);
			auto ret = static_cast<Returner*>(new ReturnValue<K_Array<Object*>>(pArr));
			ret->myType = EType::VECTOR;
			return ret;

		}
		case EType::LIST:
		{
			auto pHeadPrev = reinterpret_cast<K_Node<Object*>*>((std::byte*)scr + mem.GetOffset());
			if (pHeadPrev != nullptr)
			{
				auto pHead = pHeadPrev->pNext;
				if (pHead != nullptr)
				{
					auto pObj = pHead->data;
					K_List<Object*>* pList = new K_List<Object*>(pHead);
					auto ret = static_cast<Returner*>(new ReturnValue<K_List<Object*>>(pList));
					ret->myType = EType::LIST;
					return ret;
				}
				/*
				auto pHead = pHeadPrev->pNext;
				if (pHead != nullptr)
				{
					// ��� ����Ʈ�� ��ȸ�ϸ鼭, ���ο� ����ִ� Ŭ���� ������ �����ϴ�.
					// �̴� ���ø����̼��� ���� �ʿ��� �����Դϴ�.
					auto node = pHead;
					std::vector<std::string> listedClass;
					while (node->pNext != nullptr)
					{
						listedClass.push_back(node->data->GetName());
						node = node->pNext;
					}
					auto ret = static_cast<Returner*>(new ReturnStringVector(listedClass));
					ret->myType = EType::LIST;
					
					return ret;
				}
				*/
			}
			return nullptr;
		}
		return nullptr;
		}
	}

	template<typename T>
	static auto UnloadReturner(Returner* pRet)
	{
		return *reinterpret_cast<T*>(pRet->data);
	}

	static auto GetInitRawDataByReturner(const std::string& name, const MemberVariable& mem)
	{
		auto refleTypeInfo = STEAM->refleTypeInfo.at(name);
		auto reflInitdataVec = STEAM->refleInitData.at(name);
		size_t memsize = 0;
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			// ������ ������ ��쿡�� ������ lenth�� �ڽ��� ����Ʈ ����� ���մϴ�
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// ��� ������ ũ�Ⱑ �ٱ��� ����� ������
				{
					memsize = refleTypeInfo[i + 1].GetOffset() - mem.GetOffset();
				}
				else
				{
					memsize = reflInitdataVec.size() - mem.GetOffset();
				}
				break;
			}
		}

		std::vector<std::byte> initData(memsize);

		for (int i = 0; i < memsize; i++)
		{
			initData[i] = reflInitdataVec[mem.GetOffset() + i];
		}
		
		Returner* ret = nullptr;
		switch (mem.GetType())
		{
		case EType::STRING:
		{
			for (auto& stringData : STEAM->reflInitStringData.at(name))
			{
				if (stringData.index == mem.GetIndex())
				{
					auto string = stringData.initialString;
					ret = static_cast<Returner*>(new ReturnCopy<std::string>(string));
					ret->myType = EType::STRING;
					break;
				}
			}
			break;
		}
		case EType::INT:
		{
			int* result = reinterpret_cast<int*>(initData.data());
			ret = static_cast<Returner*>(new ReturnCopy<int>(*result));
			ret->myType = EType::INT;
			break;
		}
		case EType::FLOAT:
		{
			float* result = reinterpret_cast<float*>(initData.data());
			ret = static_cast<Returner*>(new ReturnCopy<float>(*result));
			ret->myType = EType::FLOAT;
			break;
		}
		default:
			break;
		}

		return ret;
	}

	static auto GetInitRawDataByString(const std::string& name, const MemberVariable& mem)
	{
		auto refleTypeInfo = STEAM->refleTypeInfo.at(name);
		auto reflInitdataVec = STEAM->refleInitData.at(name);
		size_t memsize = 0;
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			// ������ ������ ��쿡�� ������ lenth�� �ڽ��� ����Ʈ ����� ���մϴ�
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 < memCount)	// ��� ������ ũ�Ⱑ �ٱ��� ����� ������
				{
					memsize = refleTypeInfo[i + 1].GetOffset() - mem.GetOffset();
				}
				else
				{
					memsize = reflInitdataVec.size() - mem.GetOffset();
				}
				break;
			}
		}

		std::vector<std::byte> initData(memsize);

		for (int i = 0; i < memsize; i++)
		{
			initData[i] = reflInitdataVec[mem.GetOffset() + i];
		}

		Returner* ret = nullptr;
		switch (mem.GetType())
		{
		case EType::STRING:
		{
			for (auto& stringData : STEAM->reflInitStringData.at(name))
			{
				if (stringData.index == mem.GetIndex())
				{
					auto string = stringData.initialString;
					ret = static_cast<Returner*>(new ReturnString(string));
					ret->myType = EType::STRING;
					break;
				}
			}
			break;
		}
		case EType::INT:
		{
			int* result = reinterpret_cast<int*>(initData.data());
			ret = static_cast<Returner*>(new ReturnString(std::to_string(*result)));
			ret->myType = EType::INT;
			break;
		}
		case EType::FLOAT:
		{
			float* result = reinterpret_cast<float*>(initData.data());
			ret = static_cast<Returner*>(new ReturnString(std::to_string(*result)));
			ret->myType = EType::FLOAT;
			break;
		}
		case EType::LIST:
		{
			// �̰��� ���� ���� �ʱⰪ�� �����ϴ� ���̱� ������
			// ����Ʈ�� �ƹ��͵� �������� �ʽ��ϴ�.
			std::vector<std::string> names;
			ret = static_cast<Returner*>(new ReturnStringVector(names));
			ret->myType = EType::LIST;
			/*
			auto* result = reinterpret_cast<K_List<Object*>*>(initData.data());
			if (result != nullptr)
			{
				auto node = result->GetHead();
				while (node->pNext != nullptr)
				{
					auto data = node->data;
					names.push_back(data->GetName());
					node = node->pNext;
				}
				ret = static_cast<Returner*>(new ReturnStringVector(names));
			}
			else
			{
				// ����Ʈ�� ����ִ� ���� ���ٸ� ����� 0�� ���Ͱ� ��ȯ�˴ϴ�.
				ret = static_cast<Returner*>(new ReturnStringVector(names));
			}
			ret->myType = EType::FLOAT;
			*/
			break;
		}

		default:
			break;
		}

		return ret;
	}

	static bool CompareReturner(Returner* curr, Returner* prev)
	{
		switch(curr->GetType())
		{
		case EType::INT:
		{
			int currVal = *reinterpret_cast<int*>(curr->data);
			int prevVal = (static_cast<ReturnCopy<int>*>(prev))->value;
			if (currVal != prevVal)
			{
				std::cout << "���� ���� ���� ���� �ٸ��ϴ�. " << std::endl;
				(static_cast<ReturnCopy<int>*>(prev))->value = currVal;
				return true;
			}
			return false;
		}
		case EType::FLOAT:
		{
			float currVal = *reinterpret_cast<float*>(curr->data);
			float prevVal = (static_cast<ReturnCopy<float>*>(prev))->value;
			if (currVal != prevVal)
			{
				std::cout << "���� ���� ���� ���� �ٸ��ϴ�. " << std::endl;
				(static_cast<ReturnCopy<float>*>(prev))->value = currVal;
				return true;
			}
			return false;
		}
		case EType::STRING:
		{
			std::string currVal = *reinterpret_cast<std::string*>(curr->data);
			std::string prevVal = (static_cast<ReturnCopy<std::string>*>(prev))->value;
			if (strcmp(currVal.c_str(), prevVal.c_str()) != 0)
			{
				std::cout << "���� ���� ���� ���� �ٸ��ϴ�. " << std::endl;
				(static_cast<ReturnCopy<std::string>*>(prev))->value = currVal;
				return true;
			}
			return false;
		}

		}
		return false;
	}

	static bool CompareReturnerString(Returner* curr, Returner* prev)
	{
		auto currStr = (static_cast<ReturnString*>(curr))->dataAsString;
		auto& prevStr = (static_cast<ReturnString*>(prev))->dataAsString;
		if (strcmp(currStr.c_str(), prevStr.c_str()) != 0)
		{
			/*
			std::cout 
				<< "���� ���� ���� ���� �ٸ��ϴ�. ���� �� :" << prevStr.c_str() 
				<< "���� �� :" 
				<< currStr.c_str()
				<< std::endl;
				*/
			prevStr = currStr;
			return true;
		}
		return false;
	}

	static auto GetInitRawData(const std::string& name, const MemberVariable& mem)
	{
		auto refleTypeInfo = STEAM->refleTypeInfo.at(name);
		auto reflInitdataVec = STEAM->refleInitData.at(name);
		size_t memsize = 0;
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			// ������ ������ ��쿡�� ������ lenth�� �ڽ��� ����Ʈ ����� ���մϴ�
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// ��� ������ ũ�Ⱑ �ٱ��� ����� ������
				{
					memsize = refleTypeInfo[i + 1].GetOffset() - mem.GetOffset();
				}
				else
				{
					memsize = reflInitdataVec.size() - mem.GetOffset();
				}
				break;
			}
		}

		std::vector<std::byte> initData(memsize);
		
		for (int i = 0; i < memsize; i++)
		{
			initData[i] = reflInitdataVec[mem.GetOffset() + i];
		}
		return initData;
	}

	// MemberVariable�� ������ ���ϴ� ���� ������ ������ ��ȯ���ݴϴ�
	static auto AcccessToMember(Object* scr, const MemberVariable& mem)
	{
		_ASSERT(scr != nullptr);
		//return GetReturnByType(scr, mem);
		return GetReturnByString(scr, mem);
	}


	static Returner* AcccessToMember(Object* scr, const std::string& memName)
	{
		if (scr == nullptr) return nullptr;

		auto totalVar = STEAM->GetVariables(scr->GetName());
		auto refleTypeInfo = STEAM->refleTypeInfo.at(scr->GetName());
		size_t memsize = 0;

		MemberVariable* pNowMem = nullptr;	// �⺻�����ڰ� ���� �ε��� �����ͷ�
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			if (strcmp(totalVar[i].GetName(), memName.c_str()) == 0)
			{
				pNowMem = &totalVar[i];
			}

			// ������ ������ ��쿡�� ������ lenth�� �ڽ��� ����Ʈ ����� ���մϴ�
			if (strcmp(refleTypeInfo[i].GetName(), memName.c_str()) == 0)
			{
				if (i + 1 != memCount)	// ��� ������ ũ�Ⱑ �ٱ��� ����� ������
				{
					memsize = refleTypeInfo[i + 1].GetOffset() - pNowMem->GetOffset();
				}
				else
				{
					memsize = STEAM->refleInitData.at(scr->GetName()).size() - pNowMem->GetOffset();
				}
				break;
			}
		}

		if (pNowMem == nullptr) return nullptr;
		return GetReturnByType(scr, *pNowMem);
	}

	static Returner* AcccessToMember(Object* scr, unsigned int idx)
	{
		if (scr == nullptr) return nullptr;

		auto totalVar = STEAM->GetVariables(scr->GetName());
		auto refleTypeInfo = STEAM->refleTypeInfo.at(scr->GetName());
		size_t memsize = 0;

		MemberVariable* pNowMem = nullptr;	// �⺻�����ڰ� ���� �ε��� �����ͷ�
		if (totalVar.size() <= idx)
		{
			std::cout << "�ε����� �ʹ� Ů�ϴ�." << std::endl;
			return nullptr;
		}
		else
		{
			pNowMem = &totalVar[idx];
			(totalVar.size() == idx) ?
				memsize = STEAM->refleInitData.at(scr->GetName()).size() - totalVar[idx].GetOffset() :
				memsize = refleTypeInfo[idx + 1].GetOffset() - totalVar[idx].GetOffset();
		}

		if (pNowMem == nullptr) return nullptr;
		return GetReturnByType(scr, *pNowMem);

	}

	static Object* CreateObject(const std::string& className)
	{
		Object* pObj = RtClass::CreateObject(className);
		return pObj;
	}

	static Object* CreateReplicatedObject(const std::string& className, std::uintptr_t id)
	{
		Object* pObj = RtClass::CreateRepliObject(className, true, id);
		return pObj;
	}

	Object* CreateReplicatedObject(P_ClassInfo classInfo)
	{
		std::string className;
		for (int i = 6; i < classInfo.name.size(); i++)
		{
			className += classInfo.name.at(i);
		}
		Object* pObj = RtClass::CreateRepliObject(className, true, classInfo.id);
		pObj->isReplicated = true;
		pObj->isRoot = classInfo.flag;
		GarbageCollectionManager::GetInstance()->SetRootObject(pObj);
		return pObj;
	}

	// Ŭ������ ���ͷ� �̸������� Ÿ���� �˾ƿ��� �Լ�
	static std::vector<std::string> GetObjectTypes(const std::string&& className)
	{
		auto prefix = std::string("class ");
		prefix.append(className);
		return STEAM->GetVariableNames(prefix);
	}

	// ������Ʈ �����͸����� Ÿ���� �˷��ִ� �Լ�
	// dynamic_cast�� ���ø��� ����� �ּ�ȭ�ϱ� ���� ������ ObjectName�� ���
	static std::vector<std::string> GetObjectTypes(Object* className)
	{
		return STEAM->GetVariableNames(className->GetName());
	}

	// typeid�� ���� �ش� Ÿ���� ������ �ִ� ��� ���� �̸��������ɴϴ�
	template<typename T>
	static std::vector<std::string> GetObjectTypes()
	{
		auto prefix = std::string(typeid(T).name());
		return STEAM->GetVariableNames(prefix);
	}

	static std::vector<MemberVariable> GetObjectMembers(Object* obj)
	{
		// obj�� nullptr�̸� �� ���͸� ��ȯ�մϴ�.
		if (obj == nullptr) return std::vector<MemberVariable>();
		return STEAM->GetVariables(obj->GetName());
	}

	// � Object�� �÷��װ� Destroy ���¶��, �� �̻� �������� �ʱ� ���� nullptr�� �Ѵ�
	static void SetMemberPtrToNull(Object* obj, const MemberVariable& mem)
	{
		_ASSERT(obj != nullptr);

		auto refleTypeInfo = STEAM->refleTypeInfo.at(obj->GetName());
		size_t memsize = 0;
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			// ������ ������ ��쿡�� ������ lenth�� �ڽ��� ����Ʈ ����� ���մϴ�
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// ��� ������ ũ�Ⱑ �ٱ��� ����� ������
				{
					memsize = refleTypeInfo[i + 1].GetOffset() - mem.GetOffset();
				}
				else
				{
					memsize = STEAM->refleInitData.at(obj->GetName()).size() - mem.GetOffset();
				}
				break;
			}
		}

		void* ptr = *reinterpret_cast<void**>((std::byte*)obj + mem.GetOffset());
		ptr = nullptr;
	}

	void ChangeValueByIndex(Object* pTarget, unsigned int index, Returner* ret)
	{
		auto totalVar = STEAM->GetVariables(pTarget->GetName());
		auto refleTypeInfo = STEAM->refleTypeInfo.at(pTarget->GetName());
		size_t memsize = 0;

		MemberVariable* pNowMem = nullptr;	// �⺻�����ڰ� ���� �ε��� �����ͷ�
		if (totalVar.size() <= index)
		{
			std::cout << "�ε����� �ʹ� Ů�ϴ�." << std::endl;
			return;
		}
		else
		{
			pNowMem = &totalVar[index];
			if (totalVar.size() == index)
			{
				memsize = refleTypeInfo[index + 1].GetOffset() - totalVar[index].GetOffset();
			}
			else
			{
				memsize = STEAM->refleInitData.at(pTarget->GetName()).size() - totalVar[index].GetOffset();
			}
		}

		if (pNowMem == nullptr) return;


		auto inputVal = (static_cast<ReturnString*>(ret))->dataAsString;
		switch (pNowMem->GetType())
		{
		case EType::STRING:
		{
			// ���� ��Ʈ���� �����ؼ� ���ڸ� �߰��ϰ��� �Ѵٸ�, memsize�� �����ͼ� �� ũ�⸦ Ű���ִ��� �ؾ��ҵ�?
			auto* string = reinterpret_cast<std::string*>((std::byte*)pTarget + pNowMem->GetOffset());
			*string = inputVal;
			break;
		}
		case EType::INT:
		{
			int* integer = reinterpret_cast<int*>((std::byte*)pTarget + pNowMem->GetOffset());
			*integer =  std::stoi(inputVal);
			break;
		}
		case EType::FLOAT:
		{
			float* floatPoint = reinterpret_cast<float*>((std::byte*)pTarget + pNowMem->GetOffset());
			*floatPoint = std::stof(inputVal);
			break;
		}
		case EType::PTR:
		{
			void* ptr = *reinterpret_cast<void**>((std::byte*)pTarget + pNowMem->GetOffset());
			// nullptr�� �˻��ϰų�, �ʱ�ȭ������ �ʾ����� ���� ���Ѵ�
			break;
		}
		}

	}
};

static void ShowCurrentValue(Returner* pRet)
{
	switch (pRet->myType)
	{
	case EType::STRING:
		std::cout << " " << reinterpret_cast<std::string*>(pRet->data)->c_str() << std::endl;
		break;
	case EType::INT:
		std::cout << " " << *reinterpret_cast<int*>(pRet->data) << std::endl;
		break;
	case EType::FLOAT:
		std::cout << " " << *reinterpret_cast<float*> (pRet->data) << "f" << std::endl;
		break;
	}
};

static void ShowCurrentValueString(Returner* pRet)
{
	std::cout << " " << (static_cast<ReturnString*>(pRet))->dataAsString << std::endl;
}

// ���� ������ �ִ� ������� �����ֱ⸸ �Ѵ�
static void ShowObjectMembers(Object* obj)
{
	if (obj == nullptr) return;
	auto typeInfo = STEAM->GetReflectedTypeInfo();
	auto itor = typeInfo.find(obj->GetName());
	if (itor == typeInfo.end())
	{
		std::cout << "���÷��ǵ� ��� ������ �����ϴ�." << std::endl;
		return;
	}
	
	for (const auto& nowVar : itor->second)
	{
		std::cout << "��� ���� : ";

		auto ret = ReflectionHelper::GetInstance()->AcccessToMember(obj, nowVar);
		if (ret == nullptr)
		{
			nowVar.ShowMemeber();
			std::cout << "nullptr �Դϴ�." << std::endl;  continue;
		}
		
		if (nowVar.GetType() == PrimitiveType::PTR)
		{
			nowVar.ShowMemeber();
			[ret]()
			{
				Object* pObj = reinterpret_cast<Object*>(ret->data);
				if (pObj == nullptr)
				{
					std::cout << "nullptr �Դϴ�." << std::endl;
					return;
				}
				std::cout << "Raw ptr " << pObj->GetName() << std::endl;
				ShowObjectMembers(pObj);
			}();
		}
		else if (nowVar.GetType() == PrimitiveType::VECTOR)
		{
			nowVar.ShowMemeber2();
			auto pRet = reinterpret_cast<K_Array<Object*>*>(ret->data);
			if (pRet->IsEmpty() != true)
			{
				auto size = pRet->size();
				Object** obj = pRet->GetData();
				auto name = (*obj)->GetName();
				std::cout << "K_Array " << name << "�Դϴ�. ������ : " << size << std::endl;

				for (unsigned int i = 0; i < size; i++)
				{
					ShowObjectMembers(*(obj + i));
				}
			}
			else
			{
				std::cout << " ����ִ� K_Array �Դϴ�." << std::endl;
			}
		}
		else if (nowVar.GetType() == PrimitiveType::LIST)
		{
			nowVar.ShowMemeber();
			auto pRet = reinterpret_cast<K_Node<Object*>*>(ret->data);
			int a = 0;
		}
		else
		{
			nowVar.ShowMemeber2();
			std::cout << " : ";
			ShowCurrentValueString(ret);
		}

		// AccessToMember���� ���������� �������̹Ƿ� �����Ѵ�
		delete ret;
	}



}

// typeid�� ���� �ش� Ÿ���� ������ �ִ� ��� ���� ������ �����ɴϴ�
template<typename T>
static std::vector<MemberVariable> GetObjectMembers()
{
	auto prefix = std::string(typeid(T).name());
	return STEAM->GetVariables(prefix);
};


#define CREATE_RCLASS(className) ReflectionHelper::GetInstance()->CreateObject(className)
#define CREATE_CLASS(className) static_cast<className*>(ReflectionHelper::GetInstance()->CreateObject(#className))

// names�� ������ std::vector<std::string> ���·� �ٲ��ݴϴ�.
#define VAR_NAME(names, val) \
std::vector<std::string> names; \
typeid(val) == typeid(std::string) \
? names = ReflectionHelper.GetObjectTypes(#val) : names = reflectionHelper.GetObjectTypes(val);

#define TVAR_NAME(name, val) \
std::vector<std::string> names; \
name = reflectionHelper.GetObjectTypes<val>();


