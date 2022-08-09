#include "Replication.h"
#include "RuntimeClass.h"
#include "Packet.h"
#include "PacketProcessor.h"
#include "ReflectionHelper.h"

Replication::Replication()
{
	// ���ø����̼ǿ����� �̰� �� �̻� �� �ʿ䰡 ������ ������?
	receivePaketWorker = [&]()
	{
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
					// Ŭ������ �����޶�� ��û��
					this->ReceiveClassPacket(packet);
				}
				else if (packetType == PACKET_MEMBER)
				{
					this->ReceiveMemberPacket(packet);
				}
			}

			if (this->isEnd == true && paketPath->toServer.empty())
			{
				return;
			}
			//Sleep(0);
		}
	};
}

Replication::~Replication()
{
	// �ڷᱸ���� �ʱ�ȭ�մϴ�
	replicatedMembers.clear();

	for (auto& replvar : replVarPrevValue)
	{
		for (auto& var : replvar.second)
		{
			// �������� �Ǿ��� �ش� �����͸� �����մϴ�
			delete var.data;
		}
	}

	replVarPrevValue.clear();
}

// membervarialbe ���ʹ� ������ ������ �� ���̱� ������ �������� �ʰ� ���� �����մϴ�.
void Replication::EvaluateAndStoreVariable(std::string name, const std::vector<MemberVariable>& mems)
{
	std::vector<MemberVariable> repledVars;
	for (auto& nowMem : mems)
	{
		if (nowMem.GetReplType() == ER::REPL)
		{
			repledVars.push_back(nowMem);
		}
	}
	if (repledVars.empty() != true)
	{
		replicatedMembers.insert({ name,  repledVars });
	}
}

// ���� ������ ���ؼ� �޶����� �ִ��� Ȯ���մϴ�
void Replication::EvaluateWithPrevValue(class Object* pObj)
{
	auto reflection = ReflectionHelper::GetInstance();

	auto item = replVarPrevValue.find(pObj->objectID);
	if (item != replVarPrevValue.end())
	{
		for (const auto& vars : item->second)
		{
			// ��Ʈ�� ������ ��ȯ�� ���� ���������� �մϴ�
			auto* pObjRet = reflection->AcccessToMember(pObj, vars.memberVariable);
			auto* prevVal = vars.data;


			if (reflection->CompareReturnerString(pObjRet, prevVal))
			{
				// ���� �����尡 ���� ������������ �Ǵ��մϴ�.
				if (ThreadRegister::GetInstance()->IsThisServer(TID) == IM_SERVER)
				{
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					char dummy;
					Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
					auto test = Packet::GetInstance();
				}
				else
				{
					// Ŭ���̾�Ʈ��� ���� ��� ������ ��Ŷȭ�Ͽ� �������� �����ϴ�
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					char dummy;
					Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
					auto test = Packet::GetInstance();
				}
			}

			if (pObjRet != nullptr)
			{
				delete pObjRet;
			}
		}
	}
}


void Replication::ShowReplicatedVariables()
{
	for (auto& repled : replVarPrevValue)
	{
		auto id = repled.first;

		for (auto var : repled.second)
		{
			auto& name = var.className;
			auto currVal = var.data;

			std::string value;
			value = (static_cast<ReturnString*>(currVal))->dataAsString;

			std::cout
				<< "�ν��Ͻ� ID : " << id
				<< " Ŭ���� �̸� : " << name.c_str()
				<< " Ÿ�� : " << var.memberVariable.GetName()
				<< " ������ : " << var.memberVariable.GetTypeName().c_str()
				<< " ���簪 : " << value.c_str() << std::endl;
		}
	}
}

void Replication::ReceiveClassPacket(std::queue<char>& pk)
{
	P_ClassInfo classInfo;
	classInfo = PacketProcessor::GetInstance()->ClassDataDeserialize(pk);

	std::cout << "\n���� ��Ŷ : Ŭ���� �̸� - " << classInfo.name << " �ν��Ͻ��� ID" << classInfo.id << std::endl;

	// ���ø����̼ǵ� �������� ��� ��������, �� �߿��� �ʿ��� ���� ������ ã�Ƽ�
	// ReplicatedVariable�̶�� Ŭ������ �ʱ�ȭ�Ѵ�
	auto replMem = Replication::GetInstance()->replicatedMembers;
	auto reflection = ReflectionHelper::GetInstance();
	std::vector<ReplicatedVariable> replVec;
	for (auto elem : replMem.at(classInfo.name))
	{
		// ���������� ���� ���� String ������ �����մϴ�
		auto initRawData = reflection->GetInitRawDataByString(classInfo.name, elem);
		replVec.push_back(ReplicatedVariable(classInfo.name, initRawData, elem));
	}

	replVarPrevValue.insert({ classInfo.id , replVec });
}

void Replication::ReceiveMemberPacket(std::queue<char>& pk)
{
	P_MemberInfo memberInfo;
	memberInfo = PacketProcessor::GetInstance()->MemberDataDeserialize(pk);
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	// �� ��� ������ �޾Ƽ� �������� �˷��ش�
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////

	std::cout << "\n���� ��Ŷ : �ν��Ͻ��� ID " << memberInfo.id;

	auto& repledVal = Replication::GetInstance()->replVarPrevValue;

	auto itor = repledVal.find(memberInfo.id);
	if (itor != repledVal.end())
	{
		// ���õ� ��� ��� ������ ������ �ʿ� ���� �ٲ� �ε������� �����Ѵ�
	}
}

std::map<std::string, std::vector<MemberVariable>>& Replication::GetReplicatedMembers()
{
	return replicatedMembers;
}

void LocalReplication::EvaluatePrevValue(class Object* pObj)
{
	auto reflection = ReflectionHelper::GetInstance();

	auto item = replVarPrevValue.find(pObj->objectID);
	if (item != replVarPrevValue.end())
	{
		for (const auto& vars : item->second)
		{
			// ��Ʈ�� ������ ��ȯ�� ���� ���������� �մϴ�
			auto* pObjRet = reflection->AcccessToMember(pObj, vars.memberVariable);
			auto* prevVal = vars.data;

			auto temp = (static_cast<ReturnString*>(prevVal))->dataAsString;
			if (reflection->CompareReturnerString(pObjRet, prevVal))
			{
				char dummy;
				if (isServer == IM_SERVER)
				{
					std::cout
						<< "[����] ������ �߰� " << pObj->GetName()
						<< " ID : " << pObj->objectID
						<< "�� " << vars.memberVariable.GetName()
						<< "�� " << temp.c_str()
						<< "���� " << (static_cast<ReturnString*>(prevVal))->dataAsString.c_str()
						<< "�� ����˴ϴ�." << std::endl;


					// �������� ��� ������ ����Ǿ����Ƿ� Ŭ���̾�Ʈ���� �˷��ݴϴ�
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);

				}
				else if (isServer == IM_CLIENT)
				{
					std::cout
						<< "[Ŭ��] ������ �߰� " << pObj->GetName()
						<< " ID : " << pObj->objectID
						<< "�� " << vars.memberVariable.GetName()
						<< "�� " << temp.c_str()
						<< "���� " << (static_cast<ReturnString*>(prevVal))->dataAsString.c_str()
						<< "�� ����˴ϴ�." << std::endl;

					// ���� ����Ǿ��ٸ� �������� �˷��ݴϴ�
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					Packet::GetInstance()->AccessToServer(packet, dummy, PUSH);
				}
			}

			if (pObjRet != nullptr)
			{
				delete pObjRet;
			}
		}
	}
}

void LocalReplication::EvaluateReplicated(const struct P_ClassInfo& info)
{
	auto replMem = Replication::GetInstance()->GetReplicatedMembers();
	auto reflection = ReflectionHelper::GetInstance();
	std::vector<ReplicatedVariable> replVec;
	for (auto elem : replMem.at(info.name))
	{
		// ���������� ���� ���� String ������ �����մϴ�
		auto initRawData = reflection->GetInitRawDataByString(info.name, elem);
		replVec.push_back(ReplicatedVariable(info.name, initRawData, elem));
	}

	replVarPrevValue.insert({ info.id , replVec });
}

void LocalReplication::EvaluateReplicated(class Object* pObj)
{
	auto replMem = Replication::GetInstance()->GetReplicatedMembers();
	auto reflection = ReflectionHelper::GetInstance();
	std::vector<ReplicatedVariable> replVec;

	auto itor = replMem.find(pObj->GetName());
	if (itor == replMem.end())
		return;	// ���ø����̼� ����� �ƴϹǷ� �����մϴ�.

	bool inherited = false;
	unsigned int memberSize = 0;

	// ���� ���� ��ü�� ��� ���� ���̶� �̹� ���� ID�� �ִٸ� ���� ó���� ����� �մϴ�.
	auto inHerited = replVarPrevValue.find(pObj->objectID);
	if (inHerited != replVarPrevValue.end())
	{
		inherited = true;
		memberSize = inHerited->second.size();
	}

	for (auto elem : itor->second)
	{
		// ���������� ���� ���� String ������ �����մϴ�
		auto initRawData = reflection->GetInitRawDataByString(pObj->GetName(), elem);
		elem.IncreaseIndex(memberSize);
		replVec.push_back(ReplicatedVariable(pObj->GetName(), initRawData, elem));

		if (isServer == IM_SERVER)
		{
			std::cout << "[����]" << pObj->GetName() << "�� " << elem.GetName() << "�� ���ø����̼� �˴ϴ�." << std::endl;
		}
		else
		{
			std::cout << "[Ŭ��]" << pObj->GetName() << "�� " << elem.GetName() << "�� ���ø����̼� �˴ϴ�." << std::endl;
		}

	}

	// ���� ���� ��ü�� ��� ���� ���̶� �̹� ���� ID�� �ִٸ� ���� ó���� ����� �մϴ�.
	if (inherited)
	{
		// ��ӵ� Ŭ�����̹Ƿ�, ���� Ŭ������ ���� �������� ������ �ڽ��� ������ �����մϴ�.
		auto& superClassMem = inHerited->second;
		superClassMem.swap(replVec);
	}
	else
	{
		// ��ӵ� Ŭ������ �ƴϰų� ���� Ŭ�����Դϴ�.
		replVarPrevValue.insert({ pObj->objectID , replVec });
	}

	// ���ø����̼ǵ� Ŭ�������� ������ �����Ƿ� ��������ϴ�.
	repliClass.insert({ pObj->objectID , pObj });
}

void LocalReplication::ApplyChangedValue(std::queue<char>& packet)
{
	auto memInfo = PacketProcessor::GetInstance()->MemberDataDeserialize(packet);
	auto itor = replVarPrevValue.find(memInfo.id);
	if (itor != replVarPrevValue.end())
	{
		for (auto& mem : itor->second)
		{
			if (mem.memberVariable.GetIndex() == memInfo.index)
			{
				if (isServer == IM_SERVER)
				{
					std::cout << "[����] ���� ������ ID : " << memInfo.id << " �� ����Ǿ� ";
				}
				else
				{
					std::cout << "[Ŭ��] ���� ������ ID : " << memInfo.id << " �� ����Ǿ� ";
				}

				((ReturnString*)(mem.data))->dataAsString = memInfo.data;
				std::cout
					<< "���� ���� : " << ((ReturnString*)(mem.data))->dataAsString.c_str()
					<< "�Դϴ�."
					<< std::endl;


				// ��������� ���� ���ø����̼��� ���� ������ ����ҿ� ����� ���̰�
				// �����ε� �ٲٴ� ������ �ʿ��մϴ�.
				auto itor = repliClass.find(memInfo.id);
				if (itor != repliClass.end())
				{
					ReflectionHelper::GetInstance()->ChangeValueByIndex(itor->second, memInfo.index, mem.data);
				}

			}
		}
	}

}

std::map<uintptr_t, class Object*>& LocalReplication::GetRepliClass()
{
	return repliClass;
}
