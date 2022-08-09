#include <thread>

#include "ObjectManager.h"
#include "Replication.h"
#include "RuntimeClass.h"
#include "PacketProcessor.h"
#include "ReflectionHelper.h"
#include "ThreadRegister.h"

static std::uintptr_t POINT_ADRESS = 0;
#define PRE_ADRESS POINT_ADRESS


ObjectManager::ObjectManager()
{
	::InitializeCriticalSection(&cs);
}


ObjectManager::~ObjectManager()
{
	::DeleteCriticalSection(&cs);
}

void ObjectManager::AddObject(class Object* pObj)
{
	// 단순 Serialize를 위해 localMemory에 데이터를 올릴 때는 ObjectList.push_back을 피합니다
	if (isLocalMemory != true)
	{
		// 오브젝트 매니저를 전역적으로 사용하다보니, 문제가 발생할 수 있어 이를 막기 위해 쓰레드 ID로
		// 구분을 지었습니다. 실제로 오브젝트 매니저는 서버와 클라이언트가 별도로 존재할 거라고 생각합니다

		AddObjectToList(pObj);

		if (isServer == IM_SERVER)
		{
			// 자신이 서버이므로 클라이언트에도 오브젝트 하나를 만들 것을 '요청'합니다
			// 정확히는 카피를 의미
			// 복제된 클래스가 아닐때만 클라이언트에 생성을 요청합니다.
			if (pObj->isReplicated != true)
			{
				AddObjectOnClinet(pObj);
			}
		}
		else if (isServer == IM_CLIENT)
		{
			// 여기 들어온 쓰레드는 클라이언트이므로 서버에게 만들 것을 요청합니다
			if (pObj->isReplicated != true)
			{
				AddObjectOnServer(pObj);
			}
		}

		return;
	}
}

std::list<class Object*>& ObjectManager::GetGIDObjects(GID gid)
{
	return GetObjectList(gid);
}

void ObjectManager::ClearObjects()
{

}

// 누가 요청을 했는지 알고, 그 요청한 쪽의 메모리(라고 가정 하고)에 인스턴스 합니다.
// 패킷을 통해 요청되었다면, 이것은 복사된 것입니다
void ObjectManager::CreateObjectWithPacket(GID gid, PACKET& packet, class LocalReplication* lopl)
{
	EnterCriticalSection(&cs);

	// 패킷을 해석해서 현재 threadGroup의 리스트에 넣습니다.
	// 사실은 같은 환경에서 돌아가고 있기 때문에, 서버든 클라든 같은 힙 영역에 클래스가 인스턴스됩니다
	auto data = PacketProcessor::GetInstance()->ClassDataDeserialize(packet);

	// class ClassName 같은 형식으로 생성되므로, class를 지워줘야합니다.
	// 앞부분이 항상 6글자이므로 이를 오프셋 삼아 뒤에 클래스 이름만 남깁니다
	std::string className;
	for (int i = 6; i < data.name.size(); i++)
	{
		className += data.name.at(i);
	}

	// 리플리케이트된 클래스를 인스턴스하고, 스레드 그룹에 따라 자료구조에 담습니다.
	auto pObj = ReflectionHelper::GetInstance()->CreateReplicatedObject(className, data.id);

	// 오브젝트의 아이디는 서버에서 준 것으로 대체합니다
	//pObj->objectID = data.id;

	// 위에서 만들어진 클래스가 리플 멤버 변수를 가지고 있는지 판단하고,
	// 있다면 새롭게 id와 매핑하여 가지고 있습니다
	lopl->EvaluateReplicated(data);

	LeaveCriticalSection(&cs);
}

std::list<class Object*>& ObjectManager::GetObjectList(GID gid)
{
	auto itor = threadGroupObjectList.find(gid);
	if (itor != threadGroupObjectList.end())
	{
		return itor->second;
	}
}

void ObjectManager::AddObjectToList(std::thread::id id, class Object* pObj)
{
	// 기존에 만들어진 객체가 현재 오브젝트의 수퍼 클래스인지를 물어보는 과정입니다
	std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
	if (PRE_ADRESS == nowAdrees)
	{
		std::cout << "위 클래스를 상속 받은 " << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
	}
	else
	{
		pObj->objectID = nowAdrees;
		std::cout << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
		PRE_ADRESS = nowAdrees;

		auto gid = ThreadRegister::GetInstance()->GetGID(id);
		GetObjectList(gid).push_back(pObj);
	}
}

void ObjectManager::AddObjectToList(class Object* pObj)
{
	// 원본에게만 포인터를 값으로한 ID가 주어집니다
	// 리플리케이션된 클래스의 경우에는 식별을 위해 원본의 아이디를 넣습니다
	if (pObj->isReplicated == false)
	{
		std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
		pObj->objectID = nowAdrees;
	}

	if (isServer == IM_SERVER)
	{
		serverList.push_back(pObj);
		std::cout << "[서버]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;

	}
	else
	{
		clientList.push_back(pObj);
		std::cout << "[클라]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
	}
}


void ObjectManager::AddObjectToList(GID gid, class Object* pObj)
{
	// 원본에게만 포인터를 값으로한 ID가 주어집니다
	// 리플리케이션된 클래스의 경우에는 식별을 위해 원본의 아이디를 넣습니다
	if (pObj->isReplicated == false)
	{
		std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
		pObj->objectID = nowAdrees;
	}

	if (isServer == IM_SERVER)
	{
		serverList.push_back(pObj);
		std::cout << "[서버]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;

	}
	else
	{
		clientList.push_back(pObj);
		std::cout << "[클라]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
	}

	/*
	auto itor = threadGroupObjectList.find(gid);
	if (itor != threadGroupObjectList.end())
	{
		itor->second.push_back(pObj);
	}
	if (isServer == true)
	{
		pServerLocal->EvaluateReplicated(pObj);
	}
	*/
}

void ObjectManager::AddObjectOnServer(class Object* pObj)
{
	// 서버 쪽에다가 어떤 클래스가 만들어졌고, 그 중에 어떤 것이 리플리케이션 됐다고 알려줘야함
	// 그러기위해서는 리플리케이션을 가져오고, 생성된 클래스의 정보를 기반으로 서버에 복제될 멤버변수를 만들어달라고
	// 그럼 서버에게 ID를 넘겨주고 그 아이디와 매핑된 변수들을 기억하게 하면 되겠다

	// 리플리케이션할 필요없는 클래스를 굳이 패킷으로 만들어서 보낼 필요는 없다.
	// 패킷으로 넘어갈 클래스 이름이 리플리케이션된 클래스가 있는지 확인
	bool isReplClass = false;

	auto& replMem = Replication::GetInstance()->GetReplicatedMembers();
	for (auto& replMember : replMem)
	{
		if (strcmp(pObj->GetName(), replMember.first.c_str()) == 0)
		{
			isReplClass = true;
			break;
		}
	}
	// 클래스가 리플리케이션되지 않았다면 종료
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// 패킷에 전달
		Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
	}

	// 여기서 서버에서 알아야 할 것은
	// 어떤 클래스가 생성되었는지, 그 클래스의 인식 정보는 무엇인지, 그 클래스에서 리플되어야할 멤버 변수는 무엇인지
	// 생각해봐야할 것은 Actor같은 부모 클래스의 경우, 어떻게 처리할지를 생각해봐야함
	// 먼저 서버 쪽에 생성하게 만든다, 어차피 id는 똑같을 테니, 자식 클래스에서 여기 한 번 더 들어오면
	// 그때 리플될 변수들의 데이터 정보를 다시 날려준다
}

void ObjectManager::AddObjectOnClinet(class Object* pObj)
{
	bool isReplClass = false;
	auto& replMem = Replication::GetInstance()->GetReplicatedMembers();
	for (auto& replMember : replMem)
	{
		if (strcmp(pObj->GetName(), replMember.first.c_str()) == 0)
		{
			isReplClass = true;
			break;
		}
	}
	// 클래스가 리플리케이션되지 않았다면 종료
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// 클라이언트에게 패킷을 전달해 만들 것을 요청합니다
		Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
	}
}

void ObjectManager::Start()
{
	// 중간에 추가된 오브젝트들도 Start를 해주기 위해 queue 사용합니다.
	while (!newObjectQue.empty())
	{
		auto* pObj = newObjectQue.front();
		newObjectQue.pop();
		pObj->Start();
	}
}

void ObjectManager::Tick()
{
	//GETTLISTOBJ(list)

	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	auto objList = GetObjectList(gid);
	for (const auto& obj : objList)
	{
		obj->Tick();
		Replication::GetInstance()->EvaluateWithPrevValue(obj);
	}





	/*
	for (const auto& obj : objectList)
	{
		obj->Tick();

		// 오브젝트마다 틱이 돌았으면 이전 값과 비교해 바뀐게 있는지 확인한다
		Replication::GetInstance()->EvaluateWithPrevValue(obj);
	}

	Replication::GetInstance()->ShowReplicatedVariables();
	*/
}

void ObjectManager::Tick(class LocalReplication* pLocal)
{
	auto gid = ThreadRegister::GetInstance()->GetGID(std::this_thread::get_id());
	auto objList = GetObjectList(gid);
	for (const auto& obj : objList)
	{
		obj->Tick();
		pLocal->EvaluatePrevValue(obj);
	}

}

SingleThreadObjectManager::SingleThreadObjectManager()
{

}

SingleThreadObjectManager::~SingleThreadObjectManager()
{

}

void SingleThreadObjectManager::AddObject(class Object* pObj)
{
	// 단순 Serialize를 위해 localMemory에 데이터를 올릴 때는 ObjectList.push_back을 피합니다
	if (isLocalMemory != true)
	{
		// 오브젝트 매니저를 전역적으로 사용하다보니, 문제가 발생할 수 있어 이를 막기 위해 쓰레드 ID로
		// 구분을 지었습니다. 실제로 오브젝트 매니저는 서버와 클라이언트가 별도로 존재할 거라고 생각합니다

		AddObjectToList(pObj);
	
		auto nowAdress = std::uintptr_t(pObj);
		if (prevPtr == nowAdress) // 상속인 클래스
		{
			// 캔슬합니다.
			if (isSeverState == IM_SERVER)
				Packet::GetInstance()->CanclationClinet(prevPacketSize);
			if(isSeverState == IM_CLIENT)
				Packet::GetInstance()->CanclationSever(prevPacketSize);

			prevPtr = nowAdress;
		}
		prevPtr = nowAdress;


		if (isSeverState == IM_SERVER)
		{
			serverReplication->EvaluateReplicated(pObj);

			// 자신이 서버이므로 클라이언트에도 오브젝트 하나를 만들 것을 '요청'합니다
			// 정확히는 카피를 의미
			// 복제된 클래스가 아닐때만 클라이언트에 생성을 요청합니다.
			if (pObj->isReplicated != true)
			{
				AddObjectOnClinet(pObj);
			}
		}
		else if (isSeverState == IM_CLIENT)
		{
			clientReplication->EvaluateReplicated(pObj);

			// 여기 들어온 쓰레드는 클라이언트이므로 서버에게 만들 것을 요청합니다
			if (pObj->isReplicated != true)
			{
				AddObjectOnServer(pObj);
			}
		}

		return;
	}
}

void SingleThreadObjectManager::Start()
{
	if (isSeverState == IM_SERVER)
	{
		while(!serverNewObjectQue.empty())
		{
			auto obj = serverNewObjectQue.front();
			serverNewObjectQue.pop();
			if (obj != nullptr)
			{
				obj->Start();
			}
		}
	}
	else
	{
		while (!clientNewObjectQue.empty())
		{
			auto obj = clientNewObjectQue.front();
			clientNewObjectQue.pop();
			obj->Start();
		}
	}
}

void SingleThreadObjectManager::Tick()
{
	if (isSeverState == IM_SERVER)
	{
		for (const auto& obj : serverObjectList)
		{
			if (obj->GCflag.IsDestroy()) continue;
			obj->Tick();
		}
	}
	else
	{
		for (const auto& obj : clientObjectList)
		{
			if (obj->GCflag.IsDestroy()) continue;
			obj->Tick();
		}
	}
}

void SingleThreadObjectManager::CreateObjectWithPacket(PACKET& packet)
{
	// 사실은 같은 환경에서 돌아가고 있기 때문에, 서버든 클라든 같은 힙 영역에 클래스가 인스턴스됩니다
	auto data = PacketProcessor::GetInstance()->ClassDataDeserialize(packet);

	// class ClassName 같은 형식으로 생성되므로, class를 지워줘야합니다.
	// 앞부분이 항상 6글자이므로 이를 오프셋 삼아 뒤에 클래스 이름만 남깁니다

	// 리플리케이트된 클래스를 인스턴스하고, 스레드 그룹에 따라 자료구조에 담습니다.
	auto pObj = ReflectionHelper::GetInstance()->CreateReplicatedObject(data);

}

void SingleThreadObjectManager::AddObjectToList(class Object* pObj)
{
	// 원본에게만 포인터를 값으로한 ID가 주어집니다
	// 리플리케이션된 클래스의 경우에는 식별을 위해 원본의 아이디를 넣습니다
	std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
	if (pObj->isReplicated == false)
	{
		// 리플리케이션되지 않은 클래스만 자신의 원본 아이디를 주소값으로 할당합니다.
		// 리플리케이션된 클래스는 이미 id값이 지정되어 들어옵니다
		pObj->objectID = nowAdrees;
	}

	// 상속을 받은 클래스의 경우에는 생성자에 의해 N번 해당 프로시저가 호출되므로,
	// 기존에 저장해둔 주소값을 기준으로 상속되었는지 아닌지를 판단합니다.
	if (PRE_ADRESS == nowAdrees)
	{
		if (isSeverState == IM_SERVER)
		{
			std::cout << "[서버]위 클래스를 상속 받은 " << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
		}
		else
		{
			std::cout << "[클라]위 클래스를 상속 받은 " << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
		}
		
	}
	else
	{
		PRE_ADRESS = nowAdrees;
		if (isSeverState == IM_SERVER)
		{
			serverObjectList.push_back(pObj);
			//serverNewObjectQue.push(pObj);
			std::cout << "[서버]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
			GarbageCollectionManager::GetInstance()->SetRootObject(pObj);
		}
		else
		{
			clientObjectList.push_back(pObj);
			//clientNewObjectQue.push(pObj);
			std::cout << "[클라]" << pObj->GetName() << "이 생성되었습니다. ID : " << pObj->objectID << std::endl;
			GarbageCollectionManager::GetInstance()->SetRootObject(pObj);
		}
	}

	if (pObj->isRoot)
	{
		GarbageCollectionManager::GetInstance()->SetRootObject(pObj);
	}

}

void SingleThreadObjectManager::AddRepliObjectOnServer(class Object* pObj)
{
	bool isReplClass = false;

	auto& replMem = Replication::GetInstance()->GetReplicatedMembers();
	for (auto& replMember : replMem)
	{
		if (strcmp(pObj->GetName(), replMember.first.c_str()) == 0)
		{
			isReplClass = true;
			break;
		}
	}
	// 클래스가 리플리케이션되지 않았다면 종료
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// 패킷에 전달
		Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
	}
}

void SingleThreadObjectManager::AddRepliObjectOnClinet(class Object* pObj)
{
	bool isReplClass = false;
	auto& replMem = Replication::GetInstance()->GetReplicatedMembers();
	for (auto& replMember : replMem)
	{
		if (strcmp(pObj->GetName(), replMember.first.c_str()) == 0)
		{
			isReplClass = true;
			break;
		}
	}
	// 클래스가 리플리케이션되지 않았다면 종료
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// 클라이언트에게 패킷을 전달해 만들 것을 요청합니다
		Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
	}
}

void SingleThreadObjectManager::AddObjectOnServer(class Object* pObj)
{
	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);
	prevPacketSize = packet.size();

	char dummy;
	while (!packet.empty())
	{
		// 패킷에 전달
		Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
	}
}

void SingleThreadObjectManager::AddObjectOnClinet(class Object* pObj)
{
	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);
	prevPacketSize = packet.size();

	char dummy;
	while (!packet.empty())
	{
		// 클라이언트에게 패킷을 전달해 만들 것을 요청합니다
		Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
	}
}
