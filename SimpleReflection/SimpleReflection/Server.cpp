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


			// ������ ���� ������ �����ϴ� ��
			// ObjectTick();
			// ExamineReplicatedVariables();

			this->Tick();

			// ���⿡�� ���� ������Ʈ�� ����ؼ� ���ư��ٰ� ��������
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

	// ������ ���� �����嵵 ������ ������ ���ϱ� ������ �̷��� �־����ϴ�
	//ThreadRegister::GetInstance()->ThreadRegist(groupID, std::this_thread::get_id(), IM_SERVER);

}

void Server::Tick()
{
	
	SummonMonster();

	SingleThreadObjectManager::GetInstance()->Start();
	SingleThreadObjectManager::GetInstance()->Tick();
	
	// ������Ʈ�� ������Ʈ�� ��� ������ ����Ǹ�, �ٲ� ���� �ִ��� Ȯ���մϴ�
	// ��, �ٲ�� �ִٸ� ��Ŷȭ�Ͽ� Ŭ���̾�Ʈ���� �˸��ϴ�
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

	// ��Ŷ�� ��� �������� ���ư��ϴ�
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
		std::cout << "[����] ���͸� ��ȯ�ұ��?" << std::endl;
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
				std::cout << "[����] ���带 ã�� ���� ���͸� ��ȯ���� ���߽��ϴ�." << std::endl;
			}
		}
		else if (command.compare("n") == 0)
		{
			return;
		}
	}
	
}
