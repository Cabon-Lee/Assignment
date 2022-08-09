#pragma once

#include <string>
#include "PacketProcessor.h"
#include "OutputMemoryStream.h"
#include "Replication.h"
#include "RuntimeClass.h"

// 데이터를 받아서 패킷으로 만들어줍니다.
// 패킷에 flag에 대한 내용이 추가됩니다
PACKET PacketProcessor::ClassDataSerialize(Object* pObj)
{
	std::queue<char> data;

	// 들어가야할 것
	std::string className = pObj->GetName();
	std::uintptr_t classID = reinterpret_cast<std::uintptr_t>(pObj);

	unsigned short packetSize 
		= sizeof(unsigned short)
		+ sizeof(char)
		+ className.size()
		+ sizeof(std::uintptr_t)
		+ sizeof(char);

	unsigned char flag = pObj->isRoot;
	char nameSize = className.size();

	std::vector<char> temp(packetSize);
	memcpy(temp.data(), &packetSize, sizeof(unsigned short));
	memcpy(temp.data() + sizeof(unsigned short), &nameSize, sizeof(char));
	memcpy(temp.data() + sizeof(unsigned short) + sizeof(char), className.c_str(), className.size());
	memcpy(temp.data() 
		+ sizeof(unsigned short) 
		+ sizeof(char)
		+ className.size(), &classID, sizeof(std::uintptr_t));

	memcpy(temp.data() 
		+ sizeof(unsigned short) 
		+ sizeof(char)
		+ className.size() 
		+ sizeof(std::uintptr_t)
		, &flag, sizeof(unsigned char));

	data.push(PACKET_CLASS);
	for (auto& aByte : temp)
	{
		data.push(aByte);
	}

	return data;

	// 리플리케이션할 멤버 변수라는 것을 알려줘야 한다.
	// 생각해보니, 처음부터 서버가 리플리케이션할 멤버변수라는 걸 알고 있으면 문제가 없다
	// 그렇다면 그 리플리케이션 대상의 멤버 변수를 서버가 관리하게 하고,
	// 거기에 대한 식별 아이디를 클라이언트에게 주면 어떨까?
	// 그러니까, 클라이언트가 Player를 만들어주세요. 했다면,
	// 서버에서 이걸 만들고, 거기에 대한 pointer 주소나 기타 식별 id를 반환 값으로 주는 것이다.
	// 그럼 클라이언트에서 값이 바뀌어서 리플리케이션이 필요할 때 식별 ID로 접근해서 값을 넘겨주는게 가능하지 않을까?

	// 오브젝트를 Create했을 때, 내가 지금 클라이언트 thread라면, 이러한 오브젝트를 만들었다고 서버에 알려줘야함


	// 리플리케이션할 멤버 변수 정보
	// 패킷 형태로 바꾸어주어야할 데이터
	// 클래스 이름
	// 멤버 변수, 이름 혹은 인덱스
	// 실제 인스턴스된 객체 혹은 변수의 식별정보
	
	// 데이터를 리플리케이션할 때 필요한 정보
	// 인스턴스된 객체 혹은 변수의 식별정보
	// 타입도 알아야하나?
	// 입력할 값


}

P_ClassInfo PacketProcessor::ClassDataDeserialize(std::queue<char>& que)
{
	if (que.empty())
	{
		return P_ClassInfo();
	}

	char classNameSize = que.front();
	que.pop();

	std::string className;

	for (char i = 0; i < classNameSize; i++)
	{
		className += que.front();
		que.pop();
	}

	char temp[sizeof(std::uintptr_t)];

	for (char i = 0; i < sizeof(std::uintptr_t); i++)
	{
		if (que.empty())
		{
			int a = 0;
		}

		temp[i] = que.front();
		que.pop();
	}

	unsigned char flag = que.front();
	que.pop();

	std::uintptr_t classID;
	memcpy(&classID, temp, sizeof(std::uintptr_t));

	P_ClassInfo classInfo;
	classInfo.name = className;
	classInfo.id = classID;
	classInfo.flag = flag;

	return classInfo;
}


PACKET PacketProcessor::MemberDataSerialize(std::uintptr_t id, const ReplicatedVariable& replVar)
{
	std::queue<char> data;

	// 들어가야할 것
	auto memInfo = replVar.memberVariable;

	std::string value = (static_cast<ReturnString*>(replVar.data))->dataAsString;

	char memType = static_cast<char>(memInfo.GetType());
	char memIndex = static_cast<char>(memInfo.GetIndex());
	unsigned int dataSize = value.size();

	// 패킷 사이즈는 1, 1, 4, +@ 이므로
	// +@값이 뭐가 될지 몰라, uint_16으로 
	unsigned short packetSize 
		= sizeof(unsigned short) 
		+ sizeof(std::uintptr_t)
		+ sizeof(char) + sizeof(char) 
		+ sizeof(unsigned int) + dataSize;

	std::vector<char> temp(packetSize);
	memcpy(temp.data(), &packetSize, sizeof(unsigned short));
	memcpy(temp.data() + PACKET_OFFSET0, &id, sizeof(std::uintptr_t));
	memcpy(temp.data() + PACKET_OFFSET1, &memType, sizeof(char));
	memcpy(temp.data() + PACKET_OFFSET2, &memIndex, sizeof(char));
	memcpy(temp.data() + PACKET_OFFSET3, value.c_str(), dataSize);

	data.push(PACKET_MEMBER);
	for (auto& aByte : temp)
	{
		data.push(aByte);
	}

	return data;
}

P_MemberInfo PacketProcessor::MemberDataDeserialize(std::queue<char>& que)
{
	P_MemberInfo memberInfo;
	if (que.empty())
	{
		return memberInfo;
	}

	// 패킷의 사이즈를 판단하는 로직인데, 여기서 사용할 것은 아님 이전에 사용되어야함
	char temp[8];

	for (int i = 0; i < sizeof(std::uintptr_t); i++)
	{
		temp[i] = que.front();
		que.pop();
	}

	memberInfo.id = *reinterpret_cast<std::uintptr_t*>(temp);

	memberInfo.type = que.front();
	que.pop();

	memberInfo.index = que.front();
	que.pop();

	
	while (!que.empty())
	{
		memberInfo.data += que.front();
		que.pop();
	}

	return memberInfo;
}

PACKET PacketProcessor::RPCDataSerialize(std::uintptr_t id, char type, const class MemberFunc& memFunc)
{
	std::queue<char> data;

	auto idx = memFunc.funcIndex;
	auto name = memFunc.fucnName;
	unsigned int dataSize = name.size();

	// 패킷 사이즈는 1, 1, 4, +@ 이므로
	// +@값이 뭐가 될지 몰라, uint_16으로 
	unsigned short packetSize 
		= sizeof(unsigned short)			// 패킷 사이즈
		+ sizeof(std::uintptr_t)			// id
		+ sizeof(char)						// type
		+ sizeof(char)						// funcIndex
		+ dataSize;							// funcName

	std::vector<char> temp(packetSize);
	memcpy(temp.data(), &packetSize, sizeof(unsigned short));
	memcpy(temp.data() + sizeof(unsigned short), &id, sizeof(std::uintptr_t));
	memcpy(temp.data() + sizeof(std::uintptr_t)+ sizeof(unsigned short), &type, sizeof(char));
	memcpy(temp.data() + sizeof(std::uintptr_t)+ sizeof(unsigned short) + sizeof(char), &idx, sizeof(char));
	memcpy(temp.data() + sizeof(std::uintptr_t)+ sizeof(unsigned short) + sizeof(char) + sizeof(char), name.c_str(), dataSize);

	data.push(PACKET_RPC);
	for (auto& aByte : temp)
	{
		data.push(aByte);
	}

	return data;
}

P_PRC PacketProcessor::RPCDataDeserialize(std::queue<char>& que)
{
	P_PRC rpcInfo;
	if (que.empty())
	{
		return rpcInfo;
	}

	// 함수를 사용할 ID를 가져옴
	char temp[8];
	for (int i = 0; i < sizeof(std::uintptr_t); i++)
	{
		temp[i] = que.front();
		que.pop();
	}

	rpcInfo.id = *reinterpret_cast<std::uintptr_t*>(temp);	
	rpcInfo.type = que.front();
	que.pop();
	rpcInfo.funcIndex = que.front();
	que.pop();

	while (!que.empty())
	{
		rpcInfo.funcName += que.front();
		que.pop();
	}

	return rpcInfo;
}
