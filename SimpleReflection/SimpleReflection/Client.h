#pragma once

#include <list>
#include <functional>
#include "SimpleGame.h"

// 클라이언트의 실제 역할은 서버의 흐름에 따라서 리플리케이션된 정보들을 갱신하고
// 이것을 보여주는 역할을 합니다.
// 즉, 서버가 로직을 진짜로 가지고 있고, 클라이언트는 렌더만 담당합니다.


class Client
{
public:
	Client();
	~Client();

	void Start();
	void Tick();
	void Render();
	void PacketProcess();
	void GarbageCollection();

	std::function<void(void)> packetProcess;
	std::function<void(void)> tick;

	SimpleGame* gameLogic;

	
	bool isEnd = false;
private:
	bool isFirst = true;
	class LocalReplication* localReplication;
	unsigned char groupID;
};

