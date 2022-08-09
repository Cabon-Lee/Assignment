#include "SimpleGame.h"
#include "ReflectionHelper.h"
#include "Function.h"

#include <stdlib.h>

constexpr int FLOAT_MIN = 10;
constexpr int FLOAT_MAX = 300;

bool Monster::IsDead()
{
	if (currHP <= 0)
	{
		if (SingleThreadObjectManager::GetInstance()->isSeverState)
		{
			std::cout << "[����] ������ ü���� 0�� �Ǿ� Destory�˴ϴ�." << std::endl;
		}
		else
		{
			std::cout << "[Ŭ��] ������ ü���� 0�� �Ǿ� Destory�˴ϴ�." << std::endl;
			
		}
		DestroyThis();
		return true;
	}
	return false;
}

void Monster::Start()
{

}

void Monster::Tick()
{
	currPos = static_cast<float>(rand() % 300);


}

void SimpleGame::Start()
{
	fantasticWorld = new World(ROOT);
	hero = new Hero(ROOT);
	hero->pawnName = "Hero";
}

void SimpleGame::Tick()
{

	// Ŭ���̾�Ʈ �϶� �Ʒ��� �����մϴ�.
	if (SingleThreadObjectManager::GetInstance()->isSeverState == IM_CLIENT)
	{
		auto bullets = hero->bullets.GetHead();
		while (bullets != nullptr)
		{
			auto nowBullet = static_cast<Bullet*>(bullets->data);

			auto pawn = fantasticWorld->pawns.GetHead();
			while (pawn != nullptr)
			{
				auto monster = static_cast<Monster*>(pawn->data);
				if (nowBullet->currPos >= monster->currPos)
				{
					monster->currHP -= nowBullet->attack;
					std::cout << "[Ŭ��] �Ѿ��� �¾ҽ��ϴ�. �Ѿ� ID : " << nowBullet->objectID << " Destroy �Ǿ� ������ �ʽ��ϴ�." << std::endl;
					nowBullet->Destroy();
					if(monster->IsDead())
					{
						FunctionReplication::GetInstance()->SendRemoteProcedureCall(hero, "ExpIncrease");
					}
					break;
				}
				pawn = pawn->pNext;
			}

			bullets = bullets->pNext;
		}
	}

}


void CharacterStatus::Tick()
{
	currHealth++;
}

void World::Tick()
{
	currTempature += 0.1f;
}

void World::SummonMonster()
{
	// ���͸� ��ȯ�ϴ� �Լ��Դϴ�. �������� ȣ��˴ϴ�.
	Monster* pMonster = new Monster();
	pawns.push_back(pMonster);
}

void Hero::Tick()
{

	if (SingleThreadObjectManager::GetInstance()->isSeverState == IM_SERVER)
	{
		if (currExp >= nextExp)
		{
			std::cout << "[����] ���ΰ��� ����ġ�� �ִ�ġ�� �����Ͽ� ������ �մϴ�." << std::endl;
			FunctionReplication::GetInstance()->SendRemoteProcedureCall(this, "LevelUp");
		}
	}
	else
	{
		std::cout << "[Ŭ��] �Ѿ��� �߻��ұ��?" << std::endl;
		std::string command;
		std::cin >> command;

		if (command.compare("y") == 0)
		{
			FireBullet();
		}
	}
}

void Hero::ExpIncrease()
{
	currExp += 1.0f;
}

void Hero::LevelUp()
{
	currLevel++;
	currExp = 0.0f;
	std::cout << "���ΰ��� �������Ͽ� ���� " << currLevel << "������ �Ǿ����ϴ�." << std::endl;
}

void Hero::FireBullet()
{
	Bullet* pBullet = new Bullet();
	bullets.push_back(pBullet);
	std::cout << "[Ŭ��]�Ѿ��� �߻�Ǿ����ϴ�." << std::endl;
}

void Bullet::Tick()
{
	currPos += speed;

	if (!SingleThreadObjectManager::GetInstance()->isSeverState)
	{
		std::cout << "[Ŭ��]�Ѿ��� ��ġ " << currPos << std::endl;

		if (currPos >= limit)
		{
			std::cout << "[Ŭ��]�Ѿ��� ��ġ�� �Ѱ踦 �Ѿ� ������ϴ�." << std::endl;
			Destroy();
		}
	}
}

void Pawn::DestroyThis()
{
	this->Destroy();
	FunctionReplication::GetInstance()->SendRemoteProcedureCall(this, "Destroy");
}
