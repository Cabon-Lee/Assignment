#include "Replication.h"
#include "RuntimeClass.h"
#include "Packet.h"
#include "PacketProcessor.h"
#include "ReflectionHelper.h"

Replication::Replication()
{
	// 리플리케이션에서는 이걸 더 이상 할 필요가 없을것 같은데?
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
					// 클래스를 만들어달라는 요청임
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
	// 자료구조를 초기화합니다
	replicatedMembers.clear();

	for (auto& replvar : replVarPrevValue)
	{
		for (auto& var : replvar.second)
		{
			// 동적생성 되었던 해당 데이터를 삭제합니다
			delete var.data;
		}
	}

	replVarPrevValue.clear();
}

// membervarialbe 벡터는 어차피 참조만 할 것이기 때문에 복사하지 않고 참조 전달합니다.
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

// 값이 이전과 비교해서 달라진게 있는지 확인합니다
void Replication::EvaluateWithPrevValue(class Object* pObj)
{
	auto reflection = ReflectionHelper::GetInstance();

	auto item = replVarPrevValue.find(pObj->objectID);
	if (item != replVarPrevValue.end())
	{
		for (const auto& vars : item->second)
		{
			// 스트링 값으로 변환된 값을 가져오도록 합니다
			auto* pObjRet = reflection->AcccessToMember(pObj, vars.memberVariable);
			auto* prevVal = vars.data;


			if (reflection->CompareReturnerString(pObjRet, prevVal))
			{
				// 현재 스레드가 서버 스레드인지를 판단합니다.
				if (ThreadRegister::GetInstance()->IsThisServer(TID) == IM_SERVER)
				{
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					char dummy;
					Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);
					auto test = Packet::GetInstance();
				}
				else
				{
					// 클라이언트라면 현재 멤버 정보를 패킷화하여 서버에게 보냅니다
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
				<< "인스턴스 ID : " << id
				<< " 클래스 이름 : " << name.c_str()
				<< " 타입 : " << var.memberVariable.GetName()
				<< " 변수명 : " << var.memberVariable.GetTypeName().c_str()
				<< " 현재값 : " << value.c_str() << std::endl;
		}
	}
}

void Replication::ReceiveClassPacket(std::queue<char>& pk)
{
	P_ClassInfo classInfo;
	classInfo = PacketProcessor::GetInstance()->ClassDataDeserialize(pk);

	std::cout << "\n받은 패킷 : 클래스 이름 - " << classInfo.name << " 인스턴스된 ID" << classInfo.id << std::endl;

	// 리플리케이션된 변수들을 모두 가져오고, 그 중에서 필요한 리플 변수만 찾아서
	// ReplicatedVariable이라는 클래스를 초기화한다
	auto replMem = Replication::GetInstance()->replicatedMembers;
	auto reflection = ReflectionHelper::GetInstance();
	std::vector<ReplicatedVariable> replVec;
	for (auto elem : replMem.at(classInfo.name))
	{
		// 직관적으로 보기 쉽게 String 값으로 리턴합니다
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
	// 이 멤버 정보를 받아서 서버에게 알려준다
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////
	/////////////////////////////////////////////

	std::cout << "\n받은 패킷 : 인스턴스된 ID " << memberInfo.id;

	auto& repledVal = Replication::GetInstance()->replVarPrevValue;

	auto itor = repledVal.find(memberInfo.id);
	if (itor != repledVal.end())
	{
		// 리플된 모든 멤버 변수에 접근할 필요 없이 바뀐 인덱스에만 접근한다
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
			// 스트링 값으로 변환된 값을 가져오도록 합니다
			auto* pObjRet = reflection->AcccessToMember(pObj, vars.memberVariable);
			auto* prevVal = vars.data;

			auto temp = (static_cast<ReturnString*>(prevVal))->dataAsString;
			if (reflection->CompareReturnerString(pObjRet, prevVal))
			{
				char dummy;
				if (isServer == IM_SERVER)
				{
					std::cout
						<< "[서버] 변경점 발견 " << pObj->GetName()
						<< " ID : " << pObj->objectID
						<< "의 " << vars.memberVariable.GetName()
						<< "이 " << temp.c_str()
						<< "에서 " << (static_cast<ReturnString*>(prevVal))->dataAsString.c_str()
						<< "로 변경됩니다." << std::endl;


					// 서버에서 멤버 변수가 변경되었으므로 클라이언트에게 알려줍니다
					auto packet = PacketProcessor::GetInstance()->MemberDataSerialize(pObj->objectID, vars);
					Packet::GetInstance()->AccessToClient(packet, dummy, PUSH);

				}
				else if (isServer == IM_CLIENT)
				{
					std::cout
						<< "[클라] 변경점 발견 " << pObj->GetName()
						<< " ID : " << pObj->objectID
						<< "의 " << vars.memberVariable.GetName()
						<< "이 " << temp.c_str()
						<< "에서 " << (static_cast<ReturnString*>(prevVal))->dataAsString.c_str()
						<< "로 변경됩니다." << std::endl;

					// 값이 변경되었다면 서버에게 알려줍니다
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
		// 직관적으로 보기 쉽게 String 값으로 리턴합니다
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
		return;	// 리플리케이션 대상이 아니므로 리턴합니다.

	bool inherited = false;
	unsigned int memberSize = 0;

	// 여기 들어온 객체가 상속 받은 것이라서 이미 같은 ID가 있다면 예외 처리를 해줘야 합니다.
	auto inHerited = replVarPrevValue.find(pObj->objectID);
	if (inHerited != replVarPrevValue.end())
	{
		inherited = true;
		memberSize = inHerited->second.size();
	}

	for (auto elem : itor->second)
	{
		// 직관적으로 보기 쉽게 String 값으로 리턴합니다
		auto initRawData = reflection->GetInitRawDataByString(pObj->GetName(), elem);
		elem.IncreaseIndex(memberSize);
		replVec.push_back(ReplicatedVariable(pObj->GetName(), initRawData, elem));

		if (isServer == IM_SERVER)
		{
			std::cout << "[서버]" << pObj->GetName() << "의 " << elem.GetName() << "이 리플리케이션 됩니다." << std::endl;
		}
		else
		{
			std::cout << "[클라]" << pObj->GetName() << "의 " << elem.GetName() << "이 리플리케이션 됩니다." << std::endl;
		}

	}

	// 여기 들어온 객체가 상속 받은 것이라서 이미 같은 ID가 있다면 예외 처리를 해줘야 합니다.
	if (inherited)
	{
		// 상속된 클래스이므로, 수퍼 클래스의 이전 정보들을 저장한 자신의 정보로 스왑합니다.
		auto& superClassMem = inHerited->second;
		superClassMem.swap(replVec);
	}
	else
	{
		// 상속된 클래스가 아니거나 수퍼 클래스입니다.
		replVarPrevValue.insert({ pObj->objectID , replVec });
	}

	// 리플리케이션된 클래스들은 원본이 같으므로 상관없습니다.
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
					std::cout << "[서버] 변경 사항이 ID : " << memInfo.id << " 에 적용되어 ";
				}
				else
				{
					std::cout << "[클라] 변경 사항이 ID : " << memInfo.id << " 에 적용되어 ";
				}

				((ReturnString*)(mem.data))->dataAsString = memInfo.data;
				std::cout
					<< "현재 값이 : " << ((ReturnString*)(mem.data))->dataAsString.c_str()
					<< "입니다."
					<< std::endl;


				// 여기까지는 로컬 리플리케이션의 이전 데이터 저장소에 적용된 것이고
				// 실제로도 바꾸는 연산이 필요합니다.
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
