#include "RuntimeClass.h"

static RtClass* pHead = nullptr;

Object* RtClass::CreateObject(std::string className)
{
	for (auto it = pHead; it != nullptr; it = it->nextClass)
	{
		if (it->className.compare(className) == 0)
		{
			auto* obj = it->createObject(false, 0);
			obj->objectSize = it->getClassSize();
			return obj;
		}
	}

	printf("\n %s 는 런타임 클래스가 아닙니다. nullptr이 반환됩니다. \n", className.c_str());
	return nullptr;
}

Object* RtClass::CreateRepliObject(std::string className, bool isRepl, std::uintptr_t id)
{
	for (auto it = pHead; it != nullptr; it = it->nextClass)
	{
		if (it->className.compare(className) == 0)
		{
			auto* obj = it->createObject(isRepl, id);
			obj->objectSize = it->getClassSize();
			obj->isReplicated = isRepl;
			return obj;
		}
	}

	printf("\n %s 는 런타임 클래스가 아닙니다. nullptr이 반환됩니다. \n", className.c_str());
	return nullptr;
}

RtClass::RtClass(std::string name,	std::function<Object* (bool, std::uintptr_t)> func, std::function<size_t(void)> func2)
	: className(name)
	, createObject(func)
	, getClassSize(func2)
{
	if (pHead == nullptr)
	{
		pHead = this;
	}
	else
	{
		this->nextClass = pHead;
		pHead = this;
	}
}

RtClass::~RtClass()
{
	if (this == pHead)
	{
		pHead = pHead->nextClass;
	}
	else
	{
		for (auto prev = pHead, it = pHead->nextClass; it != nullptr; it = it->nextClass)
		{
			if (it == this)
			{
				prev->nextClass = this->nextClass;
				return;
			}
			prev = it;
		}
	}
}
