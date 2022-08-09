#include <thread>

#include "Client.h"
#include "Packet.h"
#include "ThreadRegister.h"
#include "PacketProcessor.h"
#include "ObjectManager.h"
#include "Replication.h"
#include "ReflectionHelper.h"

Client::Client()
{
	localReplication = new class LocalReplication();
	SingleThreadObjectManager::GetInstance()->clientReplication = localReplication;

	groupID = Packet::GetInstance()->groupID++;

	packetProcess = [&]()
	{
		ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_CLIENT);

		PACKET packet;
		char packetType = 0;
		while (true)
		{

			auto paketPath = Packet::GetInstance();
			while (paketPath->toClient.empty() != true)
			{
				paketPath->AccessToClient(packet, packetType, GET);
			}

			if (packet.empty() != true)
			{
				if (packetType == PACKET_CLASS)
				{

					// ��Ŷ�� ���� Ŭ������ �����, ���ø����̼��� Ȯ���� �� �ֵ��� �Ű������� �Ѱ��ݴϴ�
					ObjectManager::GetInstance()->CreateObjectWithPacket(this->groupID, packet, this->localReplication);
				}
				else if (packetType == PACKET_MEMBER)
				{
					// ������Ʈ���� ��� ��Ŷ�� ������ ��쿡��
					// �������� �ش� ��� ������ �����Ǿ����� �ǹ��մϴ�
					// ���� id������ ����� object�� �������ݴϴ�
					this->localReplication->ApplyChangedValue(packet);
				}
			}

			if (this->isEnd == true && paketPath->toClient.empty())
			{
				return;
			}
			//Sleep(0);
		}
	};


	tick = [&]()
	{
		ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_CLIENT);

		while (true)
		{
			this->Tick();

			this->Render();
		}
	};
}

Client::~Client()
{
	delete gameLogic;
}

void Client::Start()
{
	SingleThreadObjectManager::GetInstance()->isSeverState = IM_CLIENT;
	GarbageCollectionManager::GetInstance()->isSeverState = IM_CLIENT;
	FunctionReplication::GetInstance()->isStateServer = IM_CLIENT;
	if (isFirst != true) return;
	isFirst = false;
	gameLogic = new class SimpleGame();
	gameLogic->Start();
}

void Client::Tick()
{

	gameLogic->Tick();


	SingleThreadObjectManager::GetInstance()->Start();
	SingleThreadObjectManager::GetInstance()->Tick();

	/*
	// ����ΰ� ���͸� �����մϴ�.
	bool isHeroExist = false;
	bool isMonsterExist = false;
	auto objList = SingleThreadObjectManager::GetInstance()->clientObjectList;
	Object* pTarget = nullptr;
	Object* pHero = nullptr;
	for (auto pObj : objList)
	{
		if (strcmp(pObj->GetName(), "class Hero") == 0)
		{
			isHeroExist = true;
			pHero = pObj;
		}
		else if (strcmp(pObj->GetName(), "class Monster") == 0)
		{
			isMonsterExist = true;
			pTarget = pObj;
		}
	}

	if (isHeroExist && isMonsterExist)
	{
		std::cout << "[Ŭ��] ���ΰ��� ���͸� �����մϴ�." << std::endl;
		auto monster = static_cast<Monster*>(pTarget);
		std::cout << "[Ŭ��] ��� ID :" << monster->objectID;
		monster->currHP--;
		std::cout << " ���� ü�� :" << monster->currHP << std::endl;

		if (monster->currHP <= 0)
		{
			std::cout << "[Ŭ��] ���ΰ��� ���͸� �����߷Ƚ��ϴ�." << std::endl;
			monster->DestroyThis();
			FunctionReplication::GetInstance()->SendRemoteProcedureCall(pHero, "ExpIncrease");
		}
	}
	*/



	auto pObjs = SingleThreadObjectManager::GetInstance()->clientObjectList;
	for (auto pObj : pObjs)
	{
		localReplication->EvaluatePrevValue(pObj);
	}
}

void Client::Render()
{
	auto list = SingleThreadObjectManager::GetInstance()->clientObjectList;
	// �� Ŭ���� ���ο� �ִ� �������� ǥ��
	for (auto obj : list)
	{
		if (obj->GCflag.IsDestroy())
		{
			std::cout << "[Ŭ��] " << obj->GetName() << " ID : " << obj->objectID << "�� Destroy ���¶� ���̰ų� Tick() ���� �ʽ��ϴ�." << std::endl;
		}
		else
		{
			std::cout
				<< "[Ŭ��] ������ "
				<< obj->GetName() << std::endl;
			ShowObjectMembers(obj);
			std::cout << std::endl;
		}
		
	}
}

void Client::PacketProcess()
{
	PACKET packet;
	char packetType = 0;
	auto paketPath = Packet::GetInstance();
	while (paketPath->toClient.empty() != true)
	{
		paketPath->AccessToClient(packet, packetType, GET);

		if (packet.empty() != true)
		{
			if (packetType == PACKET_CLASS)
			{
				// ��Ŷ�� ���� Ŭ������ �����, ���ø����̼��� Ȯ���� �� �ֵ��� �Ű������� �Ѱ��ݴϴ�
				SingleThreadObjectManager::GetInstance()->CreateObjectWithPacket(packet);
			}
			else if (packetType == PACKET_MEMBER)
			{
				// ������Ʈ���� ��� ��Ŷ�� ������ ��쿡��
				// �������� �ش� ��� ������ �����Ǿ����� �ǹ��մϴ�
				// ���� id������ ����� object�� �������ݴϴ�
				localReplication->ApplyChangedValue(packet);
			}
			else if (packetType == PACKET_RPC)
			{
				FunctionReplication::GetInstance()->ReciveRemoteProcedureCall(packet, localReplication);
			}
		}
	}

}

void Client::GarbageCollection()
{
	GarbageCollectionManager::GetInstance()->GarbageCollect(IM_CLIENT);
}
