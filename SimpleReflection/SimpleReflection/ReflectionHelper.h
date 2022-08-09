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
		std::cout << "Reflection을 위한 Class Serialize 시작" << std::endl; 
	}
	~ReflectionHelperControl()
	{
		ObjectManager::GetInstance()->IsLocalMemory(false);
		SingleThreadObjectManager::GetInstance()->IsLocalMemory(false);
		FunctionReplication::GetInstance()->ClearObject();
		std::cout << "Reflection을 위한 Class Serialize 종료" << std::endl << std::endl;
	};
};

#define RELECTION_START() ReflectionHelperControl re;



class ReflectionHelper : public Singleton<ReflectionHelper>
{
public:
	ReflectionHelper()
	{
		
		RELECTION_START()
			// 지역 변수를 하나 만들어 해제될 때 시리얼라이저 마지막 정리를 하게 한다
			// 시리얼라이즈의 마지막 정리는 만약, 상속 받은 클래스에 대한 정리가 안끝났을 경우 이루어진다
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
			// 만약 스트링에 접근해서 문자를 추가하고자 한다면, memsize를 가져와서 더 크기를 키워주던가 해야할듯?
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
			// nullptr을 검색하거나, 초기화해주지 않았을때 값을 비교한다
			if (ptr == nullptr || (unsigned long long)ptr == 0xcdcdcdcdcdcdcdcd)
				return nullptr;
			auto ret = static_cast<Returner*>(new ReturnValue<Object>((Object*)ptr));
			ret->myType = EType::PTR;
			return ret;
		}
		case EType::VECTOR:
		{
			size_t arrSize = 0;
			// data*가 가리키는 배열의 0번쨰 원소가 pObj가 된다
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
			// nullptr을 검색하거나, 초기화해주지 않았을때 값을 비교한다
			if (ptr == nullptr || (unsigned long long)ptr == 0xcdcdcdcdcdcdcdcd)
				return nullptr;
			auto ret = static_cast<Returner*>(new ReturnValue<Object>((Object*)ptr));
			ret->myType = EType::PTR;
			return ret;
		}
		case EType::VECTOR:
		{
			size_t arrSize = 0;
			// data*가 가리키는 배열의 0번쨰 원소가 pObj가 된다
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
					// 모든 리스트를 순회하면서, 내부에 들어있는 클래스 정보를 모읍니다.
					// 이는 리플리케이션을 위해 필요한 절차입니다.
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
			// 마지막 변수의 경우에는 벡터의 lenth로 자신의 바이트 사이즈를 구합니다
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// 멤버 변수의 크기가 바깥을 벗어나지 않을때
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
			// 마지막 변수의 경우에는 벡터의 lenth로 자신의 바이트 사이즈를 구합니다
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 < memCount)	// 멤버 변수의 크기가 바깥을 벗어나지 않을때
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
			// 이곳은 이전 값을 초기값을 저장하는 곳이기 때문에
			// 리스트는 아무것도 저장하지 않습니다.
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
				// 리스트에 들어있는 것이 없다면 사이즈가 0인 벡터가 반환됩니다.
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
				std::cout << "현재 값과 이전 값이 다릅니다. " << std::endl;
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
				std::cout << "현재 값과 이전 값이 다릅니다. " << std::endl;
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
				std::cout << "현재 값과 이전 값이 다릅니다. " << std::endl;
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
				<< "현재 값과 이전 값이 다릅니다. 이전 값 :" << prevStr.c_str() 
				<< "이후 값 :" 
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
			// 마지막 변수의 경우에는 벡터의 lenth로 자신의 바이트 사이즈를 구합니다
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// 멤버 변수의 크기가 바깥을 벗어나지 않을때
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

	// MemberVariable을 넣으면 원하는 변수 정보를 참조로 반환해줍니다
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

		MemberVariable* pNowMem = nullptr;	// 기본생성자가 없어 부득이 포인터로
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			if (strcmp(totalVar[i].GetName(), memName.c_str()) == 0)
			{
				pNowMem = &totalVar[i];
			}

			// 마지막 변수의 경우에는 벡터의 lenth로 자신의 바이트 사이즈를 구합니다
			if (strcmp(refleTypeInfo[i].GetName(), memName.c_str()) == 0)
			{
				if (i + 1 != memCount)	// 멤버 변수의 크기가 바깥을 벗어나지 않을때
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

		MemberVariable* pNowMem = nullptr;	// 기본생성자가 없어 부득이 포인터로
		if (totalVar.size() <= idx)
		{
			std::cout << "인덱스가 너무 큽니다." << std::endl;
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

	// 클래스의 리터럴 이름만으로 타입을 알아오는 함수
	static std::vector<std::string> GetObjectTypes(const std::string&& className)
	{
		auto prefix = std::string("class ");
		prefix.append(className);
		return STEAM->GetVariableNames(prefix);
	}

	// 오브젝트 포인터만으로 타입을 알려주는 함수
	// dynamic_cast나 템플릿의 사용을 최소화하기 위해 내부의 ObjectName을 사용
	static std::vector<std::string> GetObjectTypes(Object* className)
	{
		return STEAM->GetVariableNames(className->GetName());
	}

	// typeid를 통해 해당 타입이 가지고 있는 멤버 변수 이름을가져옵니다
	template<typename T>
	static std::vector<std::string> GetObjectTypes()
	{
		auto prefix = std::string(typeid(T).name());
		return STEAM->GetVariableNames(prefix);
	}

	static std::vector<MemberVariable> GetObjectMembers(Object* obj)
	{
		// obj가 nullptr이면 빈 벡터를 반환합니다.
		if (obj == nullptr) return std::vector<MemberVariable>();
		return STEAM->GetVariables(obj->GetName());
	}

	// 어떤 Object의 플래그가 Destroy 상태라면, 더 이상 참조하지 않기 위해 nullptr로 한다
	static void SetMemberPtrToNull(Object* obj, const MemberVariable& mem)
	{
		_ASSERT(obj != nullptr);

		auto refleTypeInfo = STEAM->refleTypeInfo.at(obj->GetName());
		size_t memsize = 0;
		size_t memCount = refleTypeInfo.size();
		for (int i = 0; i < memCount; i++)
		{
			// 마지막 변수의 경우에는 벡터의 lenth로 자신의 바이트 사이즈를 구합니다
			if (strcmp(refleTypeInfo[i].GetName(), mem.GetName()) == 0)
			{
				if (i + 1 != memCount)	// 멤버 변수의 크기가 바깥을 벗어나지 않을때
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

		MemberVariable* pNowMem = nullptr;	// 기본생성자가 없어 부득이 포인터로
		if (totalVar.size() <= index)
		{
			std::cout << "인덱스가 너무 큽니다." << std::endl;
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
			// 만약 스트링에 접근해서 문자를 추가하고자 한다면, memsize를 가져와서 더 크기를 키워주던가 해야할듯?
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
			// nullptr을 검색하거나, 초기화해주지 않았을때 값을 비교한다
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

// 현재 가지고 있는 멤버들을 보여주기만 한다
static void ShowObjectMembers(Object* obj)
{
	if (obj == nullptr) return;
	auto typeInfo = STEAM->GetReflectedTypeInfo();
	auto itor = typeInfo.find(obj->GetName());
	if (itor == typeInfo.end())
	{
		std::cout << "리플렉션된 멤버 변수가 없습니다." << std::endl;
		return;
	}
	
	for (const auto& nowVar : itor->second)
	{
		std::cout << "멤버 변수 : ";

		auto ret = ReflectionHelper::GetInstance()->AcccessToMember(obj, nowVar);
		if (ret == nullptr)
		{
			nowVar.ShowMemeber();
			std::cout << "nullptr 입니다." << std::endl;  continue;
		}
		
		if (nowVar.GetType() == PrimitiveType::PTR)
		{
			nowVar.ShowMemeber();
			[ret]()
			{
				Object* pObj = reinterpret_cast<Object*>(ret->data);
				if (pObj == nullptr)
				{
					std::cout << "nullptr 입니다." << std::endl;
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
				std::cout << "K_Array " << name << "입니다. 사이즈 : " << size << std::endl;

				for (unsigned int i = 0; i < size; i++)
				{
					ShowObjectMembers(*(obj + i));
				}
			}
			else
			{
				std::cout << " 비어있는 K_Array 입니다." << std::endl;
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

		// AccessToMember에서 동적생성된 데이터이므로 삭제한다
		delete ret;
	}



}

// typeid를 통해 해당 타입이 가지고 있는 멤버 변수 정보를 가져옵니다
template<typename T>
static std::vector<MemberVariable> GetObjectMembers()
{
	auto prefix = std::string(typeid(T).name());
	return STEAM->GetVariables(prefix);
};


#define CREATE_RCLASS(className) ReflectionHelper::GetInstance()->CreateObject(className)
#define CREATE_CLASS(className) static_cast<className*>(ReflectionHelper::GetInstance()->CreateObject(#className))

// names를 넣으면 std::vector<std::string> 형태로 바꿔줍니다.
#define VAR_NAME(names, val) \
std::vector<std::string> names; \
typeid(val) == typeid(std::string) \
? names = ReflectionHelper.GetObjectTypes(#val) : names = reflectionHelper.GetObjectTypes(val);

#define TVAR_NAME(name, val) \
std::vector<std::string> names; \
name = reflectionHelper.GetObjectTypes<val>();


