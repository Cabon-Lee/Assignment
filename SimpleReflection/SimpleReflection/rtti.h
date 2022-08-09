#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <functional>


enum class PrimitiveType
{
	STRING = 0, INT, FLOAT, PTR, TEMPLATE, VECTOR, LIST, MAP, FUNC
};

const char* const TYPE_NAMES[static_cast<int>(PrimitiveType::FUNC)] =
{
	"string", "int", "float", "ptr*", "template", "K_Array", "list", "map"
};

enum class ReplicationType
{
	NONE = 0, REPL,
};

using EType = PrimitiveType;
using ER = ReplicationType;

static unsigned int memberIndex;

class MemberVariable
{
public:
	MemberVariable
	(
		const char* typeName, EType type, unsigned int offset, 
		ER repl = ER::NONE, const char* tempName = nullptr
	)
		: name(typeName), type(type), offset(offset), replType(repl)
	{ 
		if (tempName != nullptr)
		{
			std::string varName = name;
			std::string templateName;
			templateName.append("<").append(tempName).append(">");
			templateName.append(" ").append(varName);

			name = templateName;
		}
		index = memberIndex++;
	}

	const char* GetName() const
	{
		return name.c_str();
	}

	PrimitiveType GetType() const
	{
		return type;
	}

	ReplicationType GetReplType() const
	{
		return replType;
	}

	std::string GetTypeName() const
	{
		return TYPE_NAMES[static_cast<int>(type)];
	}

	void ShowMemeber() const
	{
		std::cout << TYPE_NAMES[static_cast<int>(type)] << " " << name.c_str() << std::endl;
	}

	void ShowMemeber2() const
	{
		std::cout << TYPE_NAMES[static_cast<int>(type)] << " " << name.c_str();
	}

	unsigned int GetOffset() const
	{
		return offset;
	}

	unsigned int GetIndex() const 
	{
		return index;
	}

	// 상속 받은 클래스라면 기존에 존재하던 멤버 변수의 크기만큼 늘려줘야 합니다
	void IncreaseIndex(unsigned int idx)
	{
		index += idx;
	}

protected:
	std::string name;
	EType type;
	unsigned int offset;
	ER replType;
	unsigned int index;
};

class DataType
{
public:
	template <typename Iterater>
	DataType(Iterater begin, Iterater end) 
		: memberVariables(begin, end) 
	{
		memberIndex = 0;
	}

	DataType(std::initializer_list<MemberVariable> memberVariable)
		: DataType(memberVariable.begin(), memberVariable.end()) 
	{
	}

	const std::vector<MemberVariable>& GetMemberVariables() const
	{
		return memberVariables;
	}

private:
	std::vector<MemberVariable> memberVariables;
};



#define DECL_STC_BEGIN static const DataType& GetDataType() \
    { \
        static const DataType dataType \
		{ \

#define DECL_STC_END }; \
return dataType; \
}\

#define DECL_REPL(className, var, ...) {#var, PrimitiveType::LIST, offsetof(className, var), __VA_ARGS__},

#define DECL_STRING(className, var, ...) {#var, PrimitiveType::STRING, offsetof(className, var), __VA_ARGS__},
#define DECL_INT(className, var, ...) {#var, PrimitiveType::INT, offsetof(className, var), __VA_ARGS__},
#define DECL_FLOAT(className, var, ...) {#var, PrimitiveType::FLOAT, offsetof(className, var), __VA_ARGS__},
#define DECL_PTR(className, var, ...) {#var, PrimitiveType::PTR, offsetof(className, var), __VA_ARGS__},

#define DECL_VECTOR(className, className2, var) {#var, PrimitiveType::VECTOR, offsetof(className, var), ReplicationType::NONE, #className2 },
#define DECL_LIST(className, className2, var) {#var, PrimitiveType::LIST, offsetof(className, var), ReplicationType::NONE, #className2 },


// 클래스 내 자료구조는 접근할 수 리플리케이션을 통해 접근하지 못하도록 합니다.
/*
#define DECL_VECTOR(className, className2, var) {#var, PrimitiveType::VECTOR, offsetof(className, var), ReplicationType::NONE, #className2 },
#define DECL_LIST(className, className2, var) {#var, PrimitiveType::LIST, offsetof(className, var), ReplicationType::NONE, #className2 },
*/
/*
#define DECL_STRING(className, var) { #var, PrimitiveType::STRING, offsetof(className, var) },
#define DECL_INT(className, var) { #var, PrimitiveType::INT, offsetof(className, var) },
#define DECL_FLOAT(className, var) { #var, PrimitiveType::FLOAT, offsetof(className, var) },
#define DECL_PTR(className, var) { #var, PrimitiveType::PTR, offsetof(className, var) },

#define DECL_VECTOR(className, className2, var) {#var, PrimitiveType::VECTOR, offsetof(className, var), #className2 },
#define DECL_LIST(className, className2, var) {#var, PrimitiveType::LIST, offsetof(className, var), #className2 },
*/