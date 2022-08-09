#pragma once

#include "assert.h"

template <typename T>
class Singleton
{
protected:
	static T* instance;

public:
	Singleton() {}
	~Singleton() {}

	static T* GetInstance()
	{
		if (instance == nullptr)
		{
			instance = new T();
		}

		assert(instance != nullptr);

		return instance;
	}

	void ReleaseInstance()
	{
		assert(instance != nullptr);

		if (instance != nullptr)
		{
			delete instance;
			instance = nullptr;
		}
	}
};

template <typename T>
T* Singleton<T>::instance = nullptr;

