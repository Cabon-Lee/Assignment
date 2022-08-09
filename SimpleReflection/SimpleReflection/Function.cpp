#include "Function.h"
#include "Packet.h"
#include "PacketProcessor.h"
#include "Replication.h"

void FunctionReplication::CallReplicatedFunction(class Object* pObj, std::string funcName)
{

	std::uintptr_t id = pObj->objectID;
	std::map<std::uintptr_t, std::vector<MemberFunc>> mappedMember;
	if (isStateServer)
		mappedMember = severMappedMemeberFunc;
	else
		mappedMember = clientMappedMemeberFunc;

	auto itor = mappedMember.find(id);
	if (itor != mappedMember.end())
	{
		for (auto& nowFunc : itor->second)
		{
			if (funcName.compare(nowFunc.fucnName) == 0)
			{
				nowFunc.funcData();
				return;
			}
		}
		std::cout << pObj->GetName() << " 에는 " << funcName.c_str() << " 라는 함수가 없습니다." << std::endl;
	}
	else
	{
		std::cout << pObj->GetName() << " 에는 " << "리플렉션된 함수가 없습니다." << std::endl;
	}
}

void FunctionReplication::CallReplicatedFunction(class Object* pObj, unsigned int idx)
{

	std::uintptr_t id = std::uintptr_t(pObj);
	std::map<std::uintptr_t, std::vector<MemberFunc>> mappedMember;
	if (isStateServer)
		mappedMember = severMappedMemeberFunc;
	else
		mappedMember = clientMappedMemeberFunc;

	auto itor = mappedMember.find(id);
	if (itor != mappedMember.end())
	{
		if (idx >= itor->second.size())
		{
			std::cout << "일치하는 함수가 없습니다." << std::endl;
			return;
		}

		auto thisFunc = itor->second[idx];
		thisFunc.funcData();
		return;
	}

}

void FunctionReplication::SendRemoteProcedureCall(class Object* pObj, std::string funcName)
{
	std::uintptr_t id = pObj->objectID;
	std::map<std::uintptr_t, std::vector<MemberFunc>> mappedMember;

	if (isStateServer)
		mappedMember = severMappedMemeberFunc;
	else
		mappedMember = clientMappedMemeberFunc;

	auto itor = mappedMember.find(id);
	if (itor != mappedMember.end())
	{
		for (auto& nowFunc : itor->second)
		{
			if (funcName.compare(nowFunc.fucnName) == 0)
			{
				auto packet = PacketProcessor::GetInstance()->RPCDataSerialize(id, FUNC_STR, nowFunc);

				char type = PACKET_RPC;
				if (isStateServer == IM_SERVER)
				{
					Packet::GetInstance()->AccessToClient(packet, type, PUSH);
				}
				else
				{
					Packet::GetInstance()->AccessToServer(packet, type, PUSH);
				}
				return;
			}
		}
		std::cout << pObj->GetName() << " 에는 " << funcName.c_str() << " 라는 함수가 없습니다." << std::endl;
	}
	else
	{
		std::cout << pObj->GetName() << " 에는 " << "리플렉션된 함수가 없습니다." << std::endl;
	}
}

void FunctionReplication::SendRemoteProcedureCall(class Object* pObj, unsigned int idx)
{
	std::uintptr_t id = std::uintptr_t(pObj);
	std::map<std::uintptr_t, std::vector<MemberFunc>> mappedMember;
	if (isStateServer)
		mappedMember = severMappedMemeberFunc;
	else
		mappedMember = clientMappedMemeberFunc;

	auto itor = mappedMember.find(id);
	if (itor != mappedMember.end())
	{
		if (idx >= itor->second.size())
		{
			std::cout << "일치하는 함수가 없습니다." << std::endl;
			return;
		}

		auto thisFunc = itor->second[idx];
		auto packet = PacketProcessor::GetInstance()->RPCDataSerialize(id, FUNC_IDX, thisFunc);

		char type = PACKET_RPC;
		if (isStateServer == IM_SERVER)
		{
			Packet::GetInstance()->AccessToClient(packet, type, PUSH);
		}
		else
		{
			Packet::GetInstance()->AccessToServer(packet, type, PUSH);
		}
		return;
	}
	else
	{
		std::cout << pObj->GetName() << " 에는 " << "리플렉션된 함수가 없습니다." << std::endl;
	}
}

void FunctionReplication::ReciveRemoteProcedureCall(std::queue<char>& packet , class LocalReplication* local)
{

	auto memFunc = PacketProcessor::GetInstance()->RPCDataDeserialize(packet);
	auto mappedClass = local->GetRepliClass();

	auto itor = mappedClass.find(memFunc.id);
	if (itor != mappedClass.end())
	{
		Object* pObj = itor->second;

		if(memFunc.funcName.compare("Destroy") == 0)
		{
			pObj->Destroy();
		}

		if (memFunc.type == FUNC_STR)
		{
			CallReplicatedFunction(pObj, memFunc.funcName);
		}
		else if (memFunc.type == FUNC_IDX)
		{
			CallReplicatedFunction(pObj, memFunc.funcIndex);
		}
	}
}
