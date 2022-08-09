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

// Replication이 서버 내에 있다고 가정하고 작성해야 할듯
// 초기값(이전) 값을 저장할 수 있는 자료구조가 필요
class ReplicatedVariable
{
public:
	ReplicatedVariable(std::string clName, Returner* pRet, MemberVariable mem) 
		: className(clName), data(pRet), memberVariable(mem) {}
	~ReplicatedVariable() {}

public:
	std::string className;
	Returner* data; // 실제 데이터를 넣는곳, 서버에서 이 값을 가지고 클라이언트와 비교한다
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

	// 클래스 정보를 받는 함수
	void ReceiveClassPacket(std::queue<char>& pk);
	void ReceiveMemberPacket(std::queue<char>& pk);
	std::function<void(void)> receivePaketWorker;
	bool isEnd = false;

	std::map<std::string, std::vector<MemberVariable>>& GetReplicatedMembers();
private:
	std::map<std::string, std::vector<MemberVariable>> replicatedMembers;
	std::map<uintptr_t, std::vector<ReplicatedVariable>> replVarPrevValue;


};


// 전역적이지 않은 지역형 리플리케이션 입니다
// 변수를 수정해주는 역할을 합니다.
// 또한 새로 생성된 클래스가 리플된 변수를 가지고 있는지 확인합니다
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


