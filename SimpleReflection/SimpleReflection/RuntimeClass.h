#pragma once

#define __RT_CLASS_H__

#include<functional>
#include<iostream>
#include "GarbageCollectionManager.h"
#include "ObjectManager.h"

// ��ŷ�� �ʿ��� �÷���, Ȯ���� �������? ���� �������?
typedef unsigned char FLAG;

const FLAG FLAG_MARKED  = 1 << 0;	// root������Ʈ�κ��� ���� �� �־��°�?
const FLAG FLAG_CHECKED = 1 << 1;	// Mark �������� üũ�� �Ǿ��°�?
const FLAG FLAG_DESTROY = 1 << 2;	// �� �÷��װ� üũ �Ǿ������� �������� ����� �ǹ̴�, ���� GC�� ȣ��Ǿ� �������� �𸣴�
const FLAG FLAG_ROOT = 1 << 3;
const FLAG FLAG_FULL = FLAG_MARKED | FLAG_CHECKED | FLAG_DESTROY | FLAG_ROOT;
class CollectionFlag
{
public:
	CollectionFlag() : flag(0) {}
	// 0b000�̸� Garbage
	inline void ResetFlag() { flag = FLAG_DESTROY & flag; }	// Destroy�� �ִٸ�, ����� 0���� ��ȯ
	inline void SetFlag(FLAG val)
	{ 
		flag |= val;
	}
	inline void Destroy() { flag ^= FLAG_DESTROY; }
	inline bool IsDestroy() { return (flag & FLAG_DESTROY); }
	inline bool IsGarbage() 
	{
		if (flag == 0)
		{
			return true;
		}
		else if (flag & FLAG_DESTROY)
		{
			return true;
		}
		else if (flag & FLAG_FULL & FLAG_DESTROY)
		{
			return true;
		}
		/*
		if (flag == 0 || FLAG_FULL & FLAG_DESTROY || flag & FLAG_FULL & FLAG_DESTROY)
		{
			return true;
		}
		*/
		return false; 
	}	
	
	unsigned char GetFlag() { return flag; }
private:
	unsigned char flag;
};

const bool ROOT = true;

class Object
{
public:
	Object() = default;
	Object(bool isroot)
	{
		isRoot = isroot;
	}
	Object(bool isRepl, std::uintptr_t id)
	{
		isReplicated = isRepl;
		objectID = id;
	}
	virtual ~Object() = default;
	void Destroy() { GCflag.Destroy(); };

	virtual void Start() = 0;
	virtual void Tick() = 0;

	size_t objectSize = 0;
	std::uintptr_t objectID;

	CollectionFlag GCflag;
	bool isReplicated =false;
	bool isRoot =false;
	bool isSeted = false;

	std::function<const char* (void)> GetName;

protected:
	virtual void FuncReplication() {};
};


class RtClass
{
public:
	//explicit RtClass(std::string name, std::function<Object* (void)> func);
	explicit RtClass(
		std::string name, 
		std::function<Object* (bool, std::uintptr_t)> 
		func, std::function<size_t (void)> func2);
	virtual ~RtClass();
	std::string className;
	std::function<Object* (bool, std::uintptr_t)> createObject;
	std::function<size_t (void)> getClassSize;
	RtClass* nextClass{ nullptr };
	static Object* CreateObject(std::string className);
	static Object* CreateRepliObject(std::string className, bool isRepl, std::uintptr_t id);
};

#define INHERIT(class_name) \
const char* partentName = #class_name; 

#define RT_CLASS(class_name) ((RtClass*)(&class_name::class##class_name))

#define DECL_RT_CLASS(class_name) \
public: \
	class_name() { GetName = []()->const char* {return typeid(class_name).name();}; \
	SingleThreadObjectManager::GetInstance()->AddObject(this); \
	FuncReplication(); }\
	class_name(bool val, std::uintptr_t id) \
	{ \
	GetName = []()->const char* \
	{	\
	return typeid(class_name).name(); \
	};	\
	isReplicated = val; \
	objectID = id; \
	SingleThreadObjectManager::GetInstance()->AddObject(this);  \
	FuncReplication(); } \
	class_name(bool val) \
	{ \
	GetName = []()->const char* \
	{	\
	return typeid(class_name).name(); \
	};	\
	isRoot = val; \
	SingleThreadObjectManager::GetInstance()->AddObject(this);  \
	FuncReplication();} \
	static const RtClass class##class_name; \
	virtual RtClass* GetRuntimeClass() const; \

// inline ó���� ���� ������ rtti.h�� RuntimeClass.h�� main.cpp�� �ִ� ����� �¹����鼭
// LINK ������ �߻��Ѵ�

/*
#define IMPL_RT_CLASS(class_name) \
	inline const RtClass class_name::class##class_name(#class_name, [](void)->Object* {return (Object*)new(std::nothrow) class_name; }); \
	inline RtClass* class_name::GetRuntimeClass() const \
	{ return RT_CLASS(class_name); }
	*/


#define IMPL_RT_CLASS(class_name) \
	inline const RtClass class_name::class##class_name(#class_name, \
	[](bool b, std::uintptr_t id)->Object* {	\
	return (Object*)new(std::nothrow) class_name(b, id);	\
	}, \
	[](void)->size_t {return sizeof(class_name);}); \
	inline RtClass* class_name::GetRuntimeClass() const \
	{ return RT_CLASS(class_name); }


