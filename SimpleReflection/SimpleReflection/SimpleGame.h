#pragma once

#include "rtti.h"
#include "Function.h"
#include "RuntimeClass.h"
#include "K_Array.h"
#include "K_List.h"

class Component : public Object
{
public:
	DECL_RT_CLASS(Component)

		std::string compName;

	virtual void Start() override {}
	virtual void Tick() override {}

	DECL_STC_BEGIN
		DECL_STRING(Component, compName)
		DECL_STC_END
};
IMPL_RT_CLASS(Component)

class CharacterStatus : public Component
{
public:
	INHERIT(Component)
		DECL_RT_CLASS(CharacterStatus)

		int maxHealth = 10;
	int currHealth = 8;
	int maxMana = 300;
	int currMana = 300;
	float attack = 1;
	float defense = 1;
	float moveSpeed = 3.14f;

	virtual void Start() override {}
	virtual void Tick() override;

	DECL_STC_BEGIN
		DECL_INT(CharacterStatus, maxHealth)
		DECL_INT(CharacterStatus, currHealth, ER::REPL)
		DECL_INT(CharacterStatus, maxMana)
		DECL_INT(CharacterStatus, currMana, ER::REPL)
		DECL_FLOAT(CharacterStatus, attack)
		DECL_FLOAT(CharacterStatus, defense)
		DECL_FLOAT(CharacterStatus, moveSpeed)
		DECL_STC_END
};
IMPL_RT_CLASS(CharacterStatus)

class Item : public Object
{
public:
	DECL_RT_CLASS(Item)

		int value = 0;
	float weight = 0;
	virtual void Start() override {}
	virtual void Tick() override {}

	DECL_STC_BEGIN
		DECL_INT(Item, value)
		DECL_FLOAT(Item, weight)
		DECL_STC_END
};
IMPL_RT_CLASS(Item)

class Inventory : public Component
{
public:
	INHERIT(Component)
		DECL_RT_CLASS(Inventory)

		int nowGold = 0;
	float nowWeight = 0;

	virtual void Start() override {}
	virtual void Tick() override {}

	K_List<Item*> items;

	DECL_STC_BEGIN
		DECL_INT(Inventory, nowGold)
		DECL_FLOAT(Inventory, nowWeight)
		DECL_LIST(Inventory, Item*, items)
		DECL_STC_END
};
IMPL_RT_CLASS(Inventory)


class Bullet : public Object
{
public:
	DECL_RT_CLASS(Bullet)

	int attack = 1;
	float currPos = 0.0f;
	float speed = 50.f;
	float limit = 200.0f;

	DECL_STC_BEGIN
		DECL_FLOAT(Bullet, currPos)
		DECL_STC_END


	virtual void Start() override {};
	virtual void Tick() override;

};
IMPL_RT_CLASS(Bullet)


class Pawn : public Object
{
public:
	DECL_RT_CLASS(Pawn)

		std::string pawnName;

	int maxHP = 0;
	int currHP = 2;

	virtual void Start() override {};
	virtual void Tick() override {};

	void DestroyThis();

	DECL_STC_BEGIN
		DECL_STRING(Pawn, pawnName, ER::REPL)
		DECL_INT(Pawn, maxHP, ER::REPL)
		DECL_INT(Pawn, currHP, ER::REPL)
		DECL_STC_END

		FUNC_BEGIN
		FUNC_VOID(Pawn, DestroyThis)
		FUNC_END
};
IMPL_RT_CLASS(Pawn)


class Monster : public Pawn
{
public:
	INHERIT(Pawn)
		DECL_RT_CLASS(Monster)

	float currPos = 0.0f;

	bool IsDead();

	DECL_STC_BEGIN
		DECL_FLOAT(Monster, currPos, ER::REPL)
		DECL_STC_END

	virtual void Start() override;
	virtual void Tick() override;
};
IMPL_RT_CLASS(Monster)

class Hero : public Pawn
{
public:
	INHERIT(Pawn)
		DECL_RT_CLASS(Hero)

		float currExp = 0.0f;
	float nextExp = 1.0f;
	int currLevel = 1;

	K_List<Bullet*> bullets;

	virtual void Start() override {};
	virtual void Tick() override;

	DECL_STC_BEGIN
		DECL_FLOAT(Hero, currExp, ER::REPL)
		DECL_LIST(Hero, Bullet*, bullets)
		DECL_STC_END

	void ExpIncrease();
	void LevelUp();
	void FireBullet();
	
	FUNC_BEGIN
	FUNC_VOID(Hero, ExpIncrease)
	FUNC_VOID(Hero, LevelUp)
	FUNC_END
};
IMPL_RT_CLASS(Hero)




class World : public Object
{
public:
	DECL_RT_CLASS(World)

	int worldIndex = 921103;
	float currTempature = 36.5f;
	std::string worldName = "MyWorld";
	K_List<Pawn*> pawns;

	virtual void Start() override {}
	virtual void Tick() override;

	int monsterCount = 0;

	void SummonMonster();

	DECL_STC_BEGIN
		DECL_INT(World, worldIndex, ER::REPL)
		DECL_FLOAT(World, currTempature, ER::REPL)
		DECL_STRING(World, worldName, ER::REPL)
		DECL_LIST(World, Pawn*, pawns)
		DECL_STC_END

		FUNC_BEGIN
		FUNC_VOID(World, SummonMonster)
		FUNC_END
};
IMPL_RT_CLASS(World)


class SimpleGame
{
public:
	void Start();
	void Tick();

private:
	World* fantasticWorld;
	Hero* hero;
};

