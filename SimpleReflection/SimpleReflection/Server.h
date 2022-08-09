#pragma once

#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <functional>

#include "Replication.h"

// ���ø����̼� �ùķ����Ϳ��� ���� ������ �� Ŭ����
// �� Ŭ�������� �� �� �ִ� ���� ���ø����̼� ����, �̴� ������ Ŭ�� �� �� �˰� �ִ� ����


class Server
{
public:
	Server();
	~Server();

	void Start();
	void Tick();
	void PacketProcess();
	void GarbageCollection();

	std::function<void(void)> packetProcess;
	std::function<void(void)> tick;

	bool isEnd = false;
	bool isFirst = true;

private:
	// ������Ʈ �Ŵ����� �����뿡 �־���� �� ������...?
	// �������� ���� �������� �˰� �־�� �ϳ�?

	void SummonMonster();

	class LocalReplication* localReplication;

	std::map<uintptr_t, std::vector<ReplicatedVariable>> replicatedMember;

	unsigned char groupID;
};

