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
			std::cout << "[서버] 몬스터의 체력이 0이 되어 Destory됩니다." << std::endl;
		}
		else
		{
			std::cout << "[클라] 몬스터의 체력이 0이 되어 Destory됩니다." << std::endl;
			
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

	// 클라이언트 일때 아래를 진행합니다.
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
					std::cout << "[클라] 총알이 맞았습니다. 총알 ID : " << nowBullet->objectID << " Destroy 되어 보이지 않습니다." << std::endl;
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
	// 몬스터를 소환하는 함수입니다. 서버에서 호출됩니다.
	Monster* pMonster = new Monster();
	pawns.push_back(pMonster);
}

void Hero::Tick()
{

	if (SingleThreadObjectManager::GetInstance()->isSeverState == IM_SERVER)
	{
		if (currExp >= nextExp)
		{
			std::cout << "[서버] 주인공의 경험치가 최대치에 도달하여 레벨업 합니다." << std::endl;
			FunctionReplication::GetInstance()->SendRemoteProcedureCall(this, "LevelUp");
		}
	}
	else
	{
		std::cout << "[클라] 총알을 발사할까요?" << std::endl;
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
	std::cout << "주인공이 레벨업하여 현재 " << currLevel << "레벨이 되었습니다." << std::endl;
}

void Hero::FireBullet()
{
	Bullet* pBullet = new Bullet();
	bullets.push_back(pBullet);
	std::cout << "[클라]총알이 발사되었습니다." << std::endl;
}

void Bullet::Tick()
{
	currPos += speed;

	if (!SingleThreadObjectManager::GetInstance()->isSeverState)
	{
		std::cout << "[클라]총알의 위치 " << currPos << std::endl;

		if (currPos >= limit)
		{
			std::cout << "[클라]총알의 위치가 한계를 넘어 사라집니다." << std::endl;
			Destroy();
		}
	}
}

void Pawn::DestroyThis()
{
	this->Destroy();
	FunctionReplication::GetInstance()->SendRemoteProcedureCall(this, "Destroy");
}
