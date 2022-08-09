#pragma once

#include <string>

#include "rtti.h"
#include "Function.h"
#include "RuntimeClass.h"
#include "K_Array.h"
#include "K_List.h"

/// <summary>
/// Ŭ���� ������� ���� �� ���� ����
/// Ŭ������ ������� ������ �����ϱ� ���� Serialize�� �� Ŭ������ sizeof�� Ȱ���մϴ�.
/// �̶�, Ŭ������ ��������� ������� ���� �����̹Ƿ� ����, �е��� �Ǿ� ���� ���� ��� ������ �߻��մϴ�.
/// ������ ��ŷ�� 1����Ʈ ������ �ϴ� ��ĵ� �ְ�����, ����� �����ϸ� �ڵ��ϴ� �ɷ� �ϰڽ��ϴ�.
/// </summary>

class Custom : public Object
{
	DECL_RT_CLASS(Custom)
		int age = -1;
	int legCount = 3;

	virtual void Start() override {}
	virtual void Tick() override {}

	K_List<Object*> list;

	DECL_STC_BEGIN
		DECL_INT(Custom, age, ER::REPL)
		DECL_INT(Custom, legCount)
		DECL_LIST(Custom, Object*, list)
		DECL_STC_END

		void DoSomething() { printf("I'm Custom"); };
	FUNC_BEGIN
	FUNC_VOID(Custom, DoSomething)
	FUNC_END
};


/*
FUNC_BEGIN
	FUNC_VOID(Custom, DoSomething)
FUNC_END
*/

IMPL_RT_CLASS(Custom)


class Actor : public Object
{
public:
	DECL_RT_CLASS(Actor)
		std::string asd = "Unreal";
	int id = 10;
	Custom* pCustom = nullptr;

	virtual void Start() override {}
	virtual void Tick() override {}


	DECL_STC_BEGIN
		DECL_STRING(Actor, asd)
		DECL_INT(Actor, id)
		DECL_PTR(Actor, pCustom)
		DECL_STC_END
};
IMPL_RT_CLASS(Actor)

class Actress : public Actor
{
public:
	INHERIT(Actor)
		DECL_RT_CLASS(Actress)
		std::string title = "Lawyer Woo";
	int age = 27;

	DECL_STC_BEGIN
		DECL_STRING(Actress, title)
		DECL_INT(Actress, age)
		DECL_STC_END
};
IMPL_RT_CLASS(Actress)

