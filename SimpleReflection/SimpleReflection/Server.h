#pragma once

#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <functional>

#include "Replication.h"

// 리플리케이션 시뮬레이터에서 서버 역할을 할 클래스
// 이 클래스에서 알 수 있는 것은 리플리케이션 정보, 이는 서버나 클라나 둘 다 알고 있는 정보


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
	// 오브젝트 매니저가 여기쯤에 있어야할 것 같은데...?
	// 서버에서 게임 엔진까지 알고 있어야 하나?

	void SummonMonster();

	class LocalReplication* localReplication;

	std::map<uintptr_t, std::vector<ReplicatedVariable>> replicatedMember;

	unsigned char groupID;
};

