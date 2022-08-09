#pragma once

#include <string>
#include "PacketProcessor.h"
#include "OutputMemoryStream.h"
#include "Replication.h"
#include "RuntimeClass.h"

// �����͸� �޾Ƽ� ��Ŷ���� ������ݴϴ�.
// ��Ŷ�� flag�� ���� ������ �߰��˴ϴ�
PACKET PacketProcessor::ClassDataSerialize(Object* pObj)
{
	std::queue<char> data;

	// ������ ��
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

	// ���ø����̼��� ��� ������� ���� �˷���� �Ѵ�.
	// �����غ���, ó������ ������ ���ø����̼��� ���������� �� �˰� ������ ������ ����
	// �׷��ٸ� �� ���ø����̼� ����� ��� ������ ������ �����ϰ� �ϰ�,
	// �ű⿡ ���� �ĺ� ���̵� Ŭ���̾�Ʈ���� �ָ� ���?
	// �׷��ϱ�, Ŭ���̾�Ʈ�� Player�� ������ּ���. �ߴٸ�,
	// �������� �̰� �����, �ű⿡ ���� pointer �ּҳ� ��Ÿ �ĺ� id�� ��ȯ ������ �ִ� ���̴�.
	// �׷� Ŭ���̾�Ʈ���� ���� �ٲ� ���ø����̼��� �ʿ��� �� �ĺ� ID�� �����ؼ� ���� �Ѱ��ִ°� �������� ������?

	// ������Ʈ�� Create���� ��, ���� ���� Ŭ���̾�Ʈ thread���, �̷��� ������Ʈ�� ������ٰ� ������ �˷������


	// ���ø����̼��� ��� ���� ����
	// ��Ŷ ���·� �ٲپ��־���� ������
	// Ŭ���� �̸�
	// ��� ����, �̸� Ȥ�� �ε���
	// ���� �ν��Ͻ��� ��ü Ȥ�� ������ �ĺ�����
	
	// �����͸� ���ø����̼��� �� �ʿ��� ����
	// �ν��Ͻ��� ��ü Ȥ�� ������ �ĺ�����
	// Ÿ�Ե� �˾ƾ��ϳ�?
	// �Է��� ��


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

	// ������ ��
	auto memInfo = replVar.memberVariable;

	std::string value = (static_cast<ReturnString*>(replVar.data))->dataAsString;

	char memType = static_cast<char>(memInfo.GetType());
	char memIndex = static_cast<char>(memInfo.GetIndex());
	unsigned int dataSize = value.size();

	// ��Ŷ ������� 1, 1, 4, +@ �̹Ƿ�
	// +@���� ���� ���� ����, uint_16���� 
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

	// ��Ŷ�� ����� �Ǵ��ϴ� �����ε�, ���⼭ ����� ���� �ƴ� ������ ���Ǿ����
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

	// ��Ŷ ������� 1, 1, 4, +@ �̹Ƿ�
	// +@���� ���� ���� ����, uint_16���� 
	unsigned short packetSize 
		= sizeof(unsigned short)			// ��Ŷ ������
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

	// �Լ��� ����� ID�� ������
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
