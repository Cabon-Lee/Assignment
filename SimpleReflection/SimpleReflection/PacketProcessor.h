#pragma once

#include <queue>
#include <map>
#include "Singleton.h"
#include "Packet.h"

using PACKET = std::queue<char>;

const unsigned int PACKET_OFFSET0 = sizeof(unsigned short);
const unsigned int PACKET_OFFSET1 = PACKET_OFFSET0 + sizeof(std::uintptr_t);
const unsigned int PACKET_OFFSET2 = PACKET_OFFSET1 + sizeof(char);
const unsigned int PACKET_OFFSET3 = PACKET_OFFSET2 + sizeof(char);

const char FUNC_IDX = 5;
const char FUNC_STR = 10;

// 요청이 들어왔을 때 데이터를 패킷으로 전환해주는 클래스
class PacketProcessor : public Singleton<PacketProcessor>
{
public:
	
	PACKET ClassDataSerialize(class Object* pObj);
	P_ClassInfo ClassDataDeserialize(std::queue<char>& que);

	PACKET MemberDataSerialize(std::uintptr_t id, const class ReplicatedVariable& replVar);
	P_MemberInfo MemberDataDeserialize(std::queue<char>& que);

	PACKET RPCDataSerialize(std::uintptr_t id, char type, const class MemberFunc& memFunc);
	P_PRC RPCDataDeserialize(std::queue<char>& que);
private:
	
};

