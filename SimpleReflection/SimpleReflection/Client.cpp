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

					// 패킷을 통해 클래스를 만들고, 리플리케이션이 확인할 수 있도록 매개변수를 넘겨줍니다
					ObjectManager::GetInstance()->CreateObjectWithPacket(this->groupID, packet, this->localReplication);
				}
				else if (packetType == PACKET_MEMBER)
				{
					// 오브젝트에서 멤버 패킷이 도착한 경우에는
					// 서버에서 해당 멤버 변수가 수정되었음을 의미합니다
					// 따라서 id값으로 변경된 object를 수정해줍니다
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
	// 히어로가 몬스터를 공격합니다.
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
		std::cout << "[클라] 주인공이 몬스터를 공격합니다." << std::endl;
		auto monster = static_cast<Monster*>(pTarget);
		std::cout << "[클라] 대상 ID :" << monster->objectID;
		monster->currHP--;
		std::cout << " 현재 체력 :" << monster->currHP << std::endl;

		if (monster->currHP <= 0)
		{
			std::cout << "[클라] 주인공이 몬스터를 쓰러뜨렸습니다." << std::endl;
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
	// 각 클래스 내부에 있는 정보들을 표현
	for (auto obj : list)
	{
		if (obj->GCflag.IsDestroy())
		{
			std::cout << "[클라] " << obj->GetName() << " ID : " << obj->objectID << "는 Destroy 상태라 보이거나 Tick() 되지 않습니다." << std::endl;
		}
		else
		{
			std::cout
				<< "[클라] 렌더링 "
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
				// 패킷을 통해 클래스를 만들고, 리플리케이션이 확인할 수 있도록 매개변수를 넘겨줍니다
				SingleThreadObjectManager::GetInstance()->CreateObjectWithPacket(packet);
			}
			else if (packetType == PACKET_MEMBER)
			{
				// 오브젝트에서 멤버 패킷이 도착한 경우에는
				// 서버에서 해당 멤버 변수가 수정되었음을 의미합니다
				// 따라서 id값으로 변경된 object를 수정해줍니다
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
