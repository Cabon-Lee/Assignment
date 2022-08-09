#pragma once

#include <string>

#include "rtti.h"
#include "Function.h"
#include "RuntimeClass.h"
#include "K_Array.h"
#include "K_List.h"

/// <summary>
/// 클래스 멤버변수 선언 시 주의 사항
/// 클래스의 멤버변수 내용을 저장하기 위해 Serialize될 때 클래스의 sizeof를 활용합니다.
/// 이때, 클래스의 멤버변수의 개수들과 비교할 예정이므로 만약, 패딩이 되어 있지 않은 경우 문제가 발생합니다.
/// 데이터 패킹을 1바이트 단위로 하는 방식도 있겠지만, 현재는 주의하며 코딩하는 걸로 하겠습니다.
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

