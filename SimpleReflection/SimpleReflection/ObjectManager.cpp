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
	// �ܼ� Serialize�� ���� localMemory�� �����͸� �ø� ���� ObjectList.push_back�� ���մϴ�
	if (isLocalMemory != true)
	{
		// ������Ʈ �Ŵ����� ���������� ����ϴٺ���, ������ �߻��� �� �־� �̸� ���� ���� ������ ID��
		// ������ �������ϴ�. ������ ������Ʈ �Ŵ����� ������ Ŭ���̾�Ʈ�� ������ ������ �Ŷ�� �����մϴ�

		AddObjectToList(pObj);

		if (isServer == IM_SERVER)
		{
			// �ڽ��� �����̹Ƿ� Ŭ���̾�Ʈ���� ������Ʈ �ϳ��� ���� ���� '��û'�մϴ�
			// ��Ȯ���� ī�Ǹ� �ǹ�
			// ������ Ŭ������ �ƴҶ��� Ŭ���̾�Ʈ�� ������ ��û�մϴ�.
			if (pObj->isReplicated != true)
			{
				AddObjectOnClinet(pObj);
			}
		}
		else if (isServer == IM_CLIENT)
		{
			// ���� ���� ������� Ŭ���̾�Ʈ�̹Ƿ� �������� ���� ���� ��û�մϴ�
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

// ���� ��û�� �ߴ��� �˰�, �� ��û�� ���� �޸�(��� ���� �ϰ�)�� �ν��Ͻ� �մϴ�.
// ��Ŷ�� ���� ��û�Ǿ��ٸ�, �̰��� ����� ���Դϴ�
void ObjectManager::CreateObjectWithPacket(GID gid, PACKET& packet, class LocalReplication* lopl)
{
	EnterCriticalSection(&cs);

	// ��Ŷ�� �ؼ��ؼ� ���� threadGroup�� ����Ʈ�� �ֽ��ϴ�.
	// ����� ���� ȯ�濡�� ���ư��� �ֱ� ������, ������ Ŭ��� ���� �� ������ Ŭ������ �ν��Ͻ��˴ϴ�
	auto data = PacketProcessor::GetInstance()->ClassDataDeserialize(packet);

	// class ClassName ���� �������� �����ǹǷ�, class�� ��������մϴ�.
	// �պκ��� �׻� 6�����̹Ƿ� �̸� ������ ��� �ڿ� Ŭ���� �̸��� ����ϴ�
	std::string className;
	for (int i = 6; i < data.name.size(); i++)
	{
		className += data.name.at(i);
	}

	// ���ø�����Ʈ�� Ŭ������ �ν��Ͻ��ϰ�, ������ �׷쿡 ���� �ڷᱸ���� ����ϴ�.
	auto pObj = ReflectionHelper::GetInstance()->CreateReplicatedObject(className, data.id);

	// ������Ʈ�� ���̵�� �������� �� ������ ��ü�մϴ�
	//pObj->objectID = data.id;

	// ������ ������� Ŭ������ ���� ��� ������ ������ �ִ��� �Ǵ��ϰ�,
	// �ִٸ� ���Ӱ� id�� �����Ͽ� ������ �ֽ��ϴ�
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
	// ������ ������� ��ü�� ���� ������Ʈ�� ���� Ŭ���������� ����� �����Դϴ�
	std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
	if (PRE_ADRESS == nowAdrees)
	{
		std::cout << "�� Ŭ������ ��� ���� " << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
	}
	else
	{
		pObj->objectID = nowAdrees;
		std::cout << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
		PRE_ADRESS = nowAdrees;

		auto gid = ThreadRegister::GetInstance()->GetGID(id);
		GetObjectList(gid).push_back(pObj);
	}
}

void ObjectManager::AddObjectToList(class Object* pObj)
{
	// �������Ը� �����͸� �������� ID�� �־����ϴ�
	// ���ø����̼ǵ� Ŭ������ ��쿡�� �ĺ��� ���� ������ ���̵� �ֽ��ϴ�
	if (pObj->isReplicated == false)
	{
		std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
		pObj->objectID = nowAdrees;
	}

	if (isServer == IM_SERVER)
	{
		serverList.push_back(pObj);
		std::cout << "[����]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;

	}
	else
	{
		clientList.push_back(pObj);
		std::cout << "[Ŭ��]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
	}
}


void ObjectManager::AddObjectToList(GID gid, class Object* pObj)
{
	// �������Ը� �����͸� �������� ID�� �־����ϴ�
	// ���ø����̼ǵ� Ŭ������ ��쿡�� �ĺ��� ���� ������ ���̵� �ֽ��ϴ�
	if (pObj->isReplicated == false)
	{
		std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
		pObj->objectID = nowAdrees;
	}

	if (isServer == IM_SERVER)
	{
		serverList.push_back(pObj);
		std::cout << "[����]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;

	}
	else
	{
		clientList.push_back(pObj);
		std::cout << "[Ŭ��]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
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
	// ���� �ʿ��ٰ� � Ŭ������ ���������, �� �߿� � ���� ���ø����̼� �ƴٰ� �˷������
	// �׷������ؼ��� ���ø����̼��� ��������, ������ Ŭ������ ������ ������� ������ ������ ��������� �����޶��
	// �׷� �������� ID�� �Ѱ��ְ� �� ���̵�� ���ε� �������� ����ϰ� �ϸ� �ǰڴ�

	// ���ø����̼��� �ʿ���� Ŭ������ ���� ��Ŷ���� ���� ���� �ʿ�� ����.
	// ��Ŷ���� �Ѿ Ŭ���� �̸��� ���ø����̼ǵ� Ŭ������ �ִ��� Ȯ��
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
	// Ŭ������ ���ø����̼ǵ��� �ʾҴٸ� ����
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// ��Ŷ�� ����
		Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
	}

	// ���⼭ �������� �˾ƾ� �� ����
	// � Ŭ������ �����Ǿ�����, �� Ŭ������ �ν� ������ ��������, �� Ŭ�������� ���õǾ���� ��� ������ ��������
	// �����غ����� ���� Actor���� �θ� Ŭ������ ���, ��� ó�������� �����غ�����
	// ���� ���� �ʿ� �����ϰ� �����, ������ id�� �Ȱ��� �״�, �ڽ� Ŭ�������� ���� �� �� �� ������
	// �׶� ���õ� �������� ������ ������ �ٽ� �����ش�
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
	// Ŭ������ ���ø����̼ǵ��� �ʾҴٸ� ����
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// Ŭ���̾�Ʈ���� ��Ŷ�� ������ ���� ���� ��û�մϴ�
		Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
	}
}

void ObjectManager::Start()
{
	// �߰��� �߰��� ������Ʈ�鵵 Start�� ���ֱ� ���� queue ����մϴ�.
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

		// ������Ʈ���� ƽ�� �������� ���� ���� ���� �ٲ�� �ִ��� Ȯ���Ѵ�
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
	// �ܼ� Serialize�� ���� localMemory�� �����͸� �ø� ���� ObjectList.push_back�� ���մϴ�
	if (isLocalMemory != true)
	{
		// ������Ʈ �Ŵ����� ���������� ����ϴٺ���, ������ �߻��� �� �־� �̸� ���� ���� ������ ID��
		// ������ �������ϴ�. ������ ������Ʈ �Ŵ����� ������ Ŭ���̾�Ʈ�� ������ ������ �Ŷ�� �����մϴ�

		AddObjectToList(pObj);
	
		auto nowAdress = std::uintptr_t(pObj);
		if (prevPtr == nowAdress) // ����� Ŭ����
		{
			// ĵ���մϴ�.
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

			// �ڽ��� �����̹Ƿ� Ŭ���̾�Ʈ���� ������Ʈ �ϳ��� ���� ���� '��û'�մϴ�
			// ��Ȯ���� ī�Ǹ� �ǹ�
			// ������ Ŭ������ �ƴҶ��� Ŭ���̾�Ʈ�� ������ ��û�մϴ�.
			if (pObj->isReplicated != true)
			{
				AddObjectOnClinet(pObj);
			}
		}
		else if (isSeverState == IM_CLIENT)
		{
			clientReplication->EvaluateReplicated(pObj);

			// ���� ���� ������� Ŭ���̾�Ʈ�̹Ƿ� �������� ���� ���� ��û�մϴ�
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
	// ����� ���� ȯ�濡�� ���ư��� �ֱ� ������, ������ Ŭ��� ���� �� ������ Ŭ������ �ν��Ͻ��˴ϴ�
	auto data = PacketProcessor::GetInstance()->ClassDataDeserialize(packet);

	// class ClassName ���� �������� �����ǹǷ�, class�� ��������մϴ�.
	// �պκ��� �׻� 6�����̹Ƿ� �̸� ������ ��� �ڿ� Ŭ���� �̸��� ����ϴ�

	// ���ø�����Ʈ�� Ŭ������ �ν��Ͻ��ϰ�, ������ �׷쿡 ���� �ڷᱸ���� ����ϴ�.
	auto pObj = ReflectionHelper::GetInstance()->CreateReplicatedObject(data);

}

void SingleThreadObjectManager::AddObjectToList(class Object* pObj)
{
	// �������Ը� �����͸� �������� ID�� �־����ϴ�
	// ���ø����̼ǵ� Ŭ������ ��쿡�� �ĺ��� ���� ������ ���̵� �ֽ��ϴ�
	std::uintptr_t nowAdrees = reinterpret_cast<std::uintptr_t>(pObj);
	if (pObj->isReplicated == false)
	{
		// ���ø����̼ǵ��� ���� Ŭ������ �ڽ��� ���� ���̵� �ּҰ����� �Ҵ��մϴ�.
		// ���ø����̼ǵ� Ŭ������ �̹� id���� �����Ǿ� ���ɴϴ�
		pObj->objectID = nowAdrees;
	}

	// ����� ���� Ŭ������ ��쿡�� �����ڿ� ���� N�� �ش� ���ν����� ȣ��ǹǷ�,
	// ������ �����ص� �ּҰ��� �������� ��ӵǾ����� �ƴ����� �Ǵ��մϴ�.
	if (PRE_ADRESS == nowAdrees)
	{
		if (isSeverState == IM_SERVER)
		{
			std::cout << "[����]�� Ŭ������ ��� ���� " << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
		}
		else
		{
			std::cout << "[Ŭ��]�� Ŭ������ ��� ���� " << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
		}
		
	}
	else
	{
		PRE_ADRESS = nowAdrees;
		if (isSeverState == IM_SERVER)
		{
			serverObjectList.push_back(pObj);
			//serverNewObjectQue.push(pObj);
			std::cout << "[����]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
			GarbageCollectionManager::GetInstance()->SetRootObject(pObj);
		}
		else
		{
			clientObjectList.push_back(pObj);
			//clientNewObjectQue.push(pObj);
			std::cout << "[Ŭ��]" << pObj->GetName() << "�� �����Ǿ����ϴ�. ID : " << pObj->objectID << std::endl;
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
	// Ŭ������ ���ø����̼ǵ��� �ʾҴٸ� ����
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// ��Ŷ�� ����
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
	// Ŭ������ ���ø����̼ǵ��� �ʾҴٸ� ����
	if (isReplClass == false) return;

	PACKET packet = PacketProcessor::GetInstance()->ClassDataSerialize(pObj);

	char dummy;
	while (!packet.empty())
	{
		// Ŭ���̾�Ʈ���� ��Ŷ�� ������ ���� ���� ��û�մϴ�
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
		// ��Ŷ�� ����
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
		// Ŭ���̾�Ʈ���� ��Ŷ�� ������ ���� ���� ��û�մϴ�
		Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
	}
}
