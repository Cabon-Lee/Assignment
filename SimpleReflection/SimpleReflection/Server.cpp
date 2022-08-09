#include <windows.h>
#include <functional>
#include <string>
#include "Server.h"
#include "Packet.h"
#include "PacketProcessor.h"
#include "ReflectionHelper.h"
#include "ObjectManager.h"
#include "ThreadRegister.h"

Server::Server()
{
	groupID = Packet::GetInstance()->groupID++;
	localReplication = new class LocalReplication();
	localReplication->isServer = IM_SERVER;
	SingleThreadObjectManager::GetInstance()->serverReplication = localReplication;
	

	auto re = ReflectionHelper::GetInstance();
	//ObjectManager::GetInstance()->pServerLocal = localReplication;

	packetProcess = [&]()
	{
		ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_SERVER);

		PACKET packet;
		char packetType = 0;
		while (true)
		{
			auto paketPath = Packet::GetInstance();
			while (paketPath->toServer.empty() != true)
			{
				paketPath->AccessToServer(packet, packetType, GET);
			}

			if (packet.empty() != true)
			{
				if (packetType == PACKET_CLASS)
				{
					ObjectManager::GetInstance()->CreateObjectWithPacket
					(this->groupID, packet, this->localReplication);
				}
				else if (packetType == PACKET_MEMBER)
				{
					this->localReplication->ApplyChangedValue(packet);
				}
			}

			if (this->isEnd == true && paketPath->toServer.empty())
			{
				return;
			}
		}
	};


	tick = [&]()
	{
		ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_SERVER);

		while (true)
		{


			// 게임의 실제 구현이 존재하는 곳
			// ObjectTick();
			// ExamineReplicatedVariables();

			this->Tick();

			// 여기에서 게임 스테이트가 계속해서 돌아간다고 생각하자
			// GameStateUpdate();

			/*
			std::string query;
			std::cin >> query;

			if (strcmp(query.c_str(), "n") == 0)
			{
				return;
			}
			*/
		}
	};
}


Server::~Server()
{
	
}

void Server::Start()
{
	SingleThreadObjectManager::GetInstance()->isSeverState = IM_SERVER;
	GarbageCollectionManager::GetInstance()->isSeverState = IM_SERVER;
	FunctionReplication::GetInstance()->isStateServer = IM_SERVER;
	if (isFirst != true) return;
	isFirst = false;

	// 어차피 메인 스레드도 서버의 역할을 겸하기 때문에 이렇게 넣었습니다
	//ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_SERVER);

}

void Server::Tick()
{
	
	SummonMonster();

	SingleThreadObjectManager::GetInstance()->Start();
	SingleThreadObjectManager::GetInstance()->Tick();
	
	// 오브젝트의 업데이트와 모든 로직이 종료되면, 바뀐 것이 있는지 확인합니다
	// 또, 바뀐게 있다면 패킷화하여 클라이언트에게 알립니다
	auto pObjs = SingleThreadObjectManager::GetInstance()->serverObjectList;
	for (auto pObj : pObjs)
	{
		localReplication->EvaluatePrevValue(pObj);
	}
}

void Server::PacketProcess()
{
	PACKET packet;
	char packetType = 0;

	auto paketPath = Packet::GetInstance();

	// 패킷이 모두 빌때까지 돌아갑니다
	while (paketPath->toServer.empty() != true)
	{
		paketPath->AccessToServer(packet, packetType, GET);

		if (packet.empty() != true)
		{
			if (packetType == PACKET_CLASS)
			{
				SingleThreadObjectManager::GetInstance()->CreateObjectWithPacket(packet);
			}
			else if (packetType == PACKET_MEMBER)
			{
				this->localReplication->ApplyChangedValue(packet);
			}
			else if (packetType == PACKET_RPC)
			{
				FunctionReplication::GetInstance()->ReciveRemoteProcedureCall(packet, localReplication);
			}
		}
	}
}

void Server::GarbageCollection()
{
	GarbageCollectionManager::GetInstance()->GarbageCollect(IM_SERVER);
}

void Server::SummonMonster()
{
	while (true)
	{
		std::cout << "[서버] 몬스터를 소환할까요?" << std::endl;
		std::string command;
		std::cin >> command;

		if (command.compare("y") == 0)
		{
			bool isSuccess = false;
			auto objList = SingleThreadObjectManager::GetInstance()->serverObjectList;
			for (auto& obj : objList)
			{
				if (strcmp(obj->GetName(), "class World") == 0)
				{
					FunctionReplication::GetInstance()->SendRemoteProcedureCall(obj, "SummonMonster");
					isSuccess = true;
					break;
				}
			}
			if (!isSuccess)
			{
				std::cout << "[서버] 월드를 찾지 못해 몬스터를 소환하지 못했습니다." << std::endl;
			}
		}
		else if (command.compare("n") == 0)
		{
			return;
		}
	}
	
}
