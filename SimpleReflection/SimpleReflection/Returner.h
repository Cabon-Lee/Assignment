#pragma once

#include "rtti.h"

class Returner
{
public:
	virtual EType GetType() { return myType; };
	EType myType;

	void* data;
};

template<typename T>
class ReturnValue : public Returner
{
public:
	ReturnValue(T* val) 
	{ 
		data = val; 
	}

	virtual EType GetType() override { return myType; };
	T* type;
};

template<typename T>
class ReturnCopy : public Returner
{
public:
	ReturnCopy(T val)
	{
		value = val;
	}

	T value;
};


class ReturnString : public Returner
{
public:
	ReturnString(std::string data) : dataAsString(data) {}

	std::string dataAsString;
};

class ReturnStringVector : public Returner
{
public:	
	ReturnStringVector(std::vector<std::string> names)
		: classNamse(names) {};
	std::vector<std::string> classNamse;
};