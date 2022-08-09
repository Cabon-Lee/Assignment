#pragma once

#include <list>
#include <functional>
#include "SimpleGame.h"

// Ŭ���̾�Ʈ�� ���� ������ ������ �帧�� ���� ���ø����̼ǵ� �������� �����ϰ�
// �̰��� �����ִ� ������ �մϴ�.
// ��, ������ ������ ��¥�� ������ �ְ�, Ŭ���̾�Ʈ�� ������ ����մϴ�.


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

