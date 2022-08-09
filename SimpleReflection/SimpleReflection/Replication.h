#pragma once

#include "Singleton.h"
#include <map>
#include <string>
#include <vector>
#include <queue>
#include <functional>
#include "rtti.h"
#include "Returner.h"
#include "Function.h"

// Replication�� ���� ���� �ִٰ� �����ϰ� �ۼ��ؾ� �ҵ�
// �ʱⰪ(����) ���� ������ �� �ִ� �ڷᱸ���� �ʿ�
class ReplicatedVariable
{
public:
	ReplicatedVariable(std::string clName, Returner* pRet, MemberVariable mem) 
		: className(clName), data(pRet), memberVariable(mem) {}
	~ReplicatedVariable() {}

public:
	std::string className;
	Returner* data; // ���� �����͸� �ִ°�, �������� �� ���� ������ Ŭ���̾�Ʈ�� ���Ѵ�
	MemberVariable memberVariable;
};

class Replication : public Singleton<Replication>
{
public:
	Replication();
	~Replication();

	void EvaluateAndStoreVariable(std::string name, const std::vector<MemberVariable>& mems);
	void EvaluateWithPrevValue(class Object* pObj);
	void ShowReplicatedVariables();

	// Ŭ���� ������ �޴� �Լ�
	void ReceiveClassPacket(std::queue<char>& pk);
	void ReceiveMemberPacket(std::queue<char>& pk);
	std::function<void(void)> receivePaketWorker;
	bool isEnd = false;

	std::map<std::string, std::vector<MemberVariable>>& GetReplicatedMembers();
private:
	std::map<std::string, std::vector<MemberVariable>> replicatedMembers;
	std::map<uintptr_t, std::vector<ReplicatedVariable>> replVarPrevValue;


};


// ���������� ���� ������ ���ø����̼� �Դϴ�
// ������ �������ִ� ������ �մϴ�.
// ���� ���� ������ Ŭ������ ���õ� ������ ������ �ִ��� Ȯ���մϴ�
class LocalReplication
{
public:
	LocalReplication() {};
	~LocalReplication() {};

public:
	void EvaluatePrevValue(class Object* pObj);
	void EvaluateReplicated(class Object* pObj);
	void EvaluateReplicated(const struct P_ClassInfo& info);
	void ApplyChangedValue(std::queue<char>& data);
	std::map<uintptr_t, class Object*>& GetRepliClass();

	bool isServer = false;
private:
	std::map<uintptr_t, std::vector<ReplicatedVariable>> replVarPrevValue;
	std::map<uintptr_t, class Object*> repliClass;


public:

};


