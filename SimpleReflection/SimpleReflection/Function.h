#pragma once

#include <windows.h>
#include <string>
#include <iostream>
#include <tuple>
#include <utility>
#include <functional>
#include <type_traits>
#include <map>
#include <vector>
#include <any>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>
#include "Replication.h"
#include "RuntimeClass.h"


#define FUCNTION(className, FucnName, ...) \
std::tuple<__VA_ARGS__> FucnName;\
auto tSize = std::tuple_size<decltype(FucnName)>::value;	\
std::function<void()> fuc = &className.FucnName;

// 매개변수의 개수를 알아낼 수 있게됨


template <typename T, typename... Ts>
struct Index;

template <typename T, typename... Ts>
struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename U, typename... Ts>
struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

template <typename T, typename... Ts>
constexpr std::size_t Index_v = Index<T, Ts...>::value;

using AnyFunc = std::function<std::any(std::vector<std::any>)>;



template <typename TOut, typename TArgsTuplePacked, std::size_t... Indices>
TOut to_dynamic_function_helper(
	const std::function<TOut(typename std::tuple_element<Indices, TArgsTuplePacked>::type...)>&
	function,
	std::vector<std::any> arguments,
	std::integer_sequence<std::size_t, Indices...>)
{
	return
		function(std::any_cast<typename std::tuple_element<Indices, TArgsTuplePacked>::type>(
			arguments[Indices])...);
}


// 어떤 형태의 함수가 됐든 들어가면 std::function<std::any(std::vector<std::any>)>  
template <typename TOut, typename... TArgs>
std::function<std::any(std::vector<std::any>)>
to_dynamic_function(std::function<TOut(TArgs...)> func)
{
	return
		[func]
	(std::vector<std::any> arguments)
	{
		if (sizeof...(TArgs) != arguments.size())
		{
			throw std::invalid_argument{ "The number of arguments does not match" };
		}

		return to_dynamic_function_helper<TOut, std::tuple<TArgs...>>
			(
				func,
				std::move(arguments),
				std::make_integer_sequence<std::size_t, sizeof...(TArgs)>()
				);
	};
}

class MemberFunc
{
public:
	MemberFunc(unsigned int idx, const char* name, std::function<void()> func)
		: funcIndex(idx), fucnName(name), funcData(func) {};

public:
	unsigned int funcIndex;
	std::string fucnName;
	std::function<void()> funcData;
};

/*
class Function
{
public:

	// Function() 이 부분과
	// DoSomething()을 추상화하는 클래스가 하나 더 존재해야함
	// 마치 타입을 리플렉션하듯이
	Function() 
	{
		// 바인드 동작은 실제 인스턴스가 생성되고 나서 진행이 되어야함

		func = std::bind(&Function::foo, this);
		funcMap.insert({"first", func});
	};



	void DoSomething(std::string fucnName)
	{
		auto itor = funcMap.find(fucnName);
		if (itor != funcMap.end())
		{
			itor->second();
		}
	}

	void foo() 
	{ 
		printf("IM FUNCTION");
	};



	std::function<void()> func;
	std::function<void(std::vector<MemberFunc>&)> start= [this](std::vector<MemberFunc>& out)
	{
		//std::vector<MemberFunc> memberVec;
		out.emplace_back("DoSomthing", std::bind(&Function::foo, static_cast<Function*>(this)));

		std::function<double(int, double)> 
			static_function
		{
			[](int a, double b) { return a * b; },
		};
		AnyFunc nowFunc = to_dynamic_function(static_function);

	};


	// 원하는 클래스에서 부르고 싶다면, 이 부분을 인스턴스의 ID와 같이 매핑해주면 될듯?
	std::map<std::string, std::function<void()>> funcMap;

};
*/

class FunctionReplication : public Singleton<FunctionReplication>
{
public:
	bool isStateServer = false;
	void ClearObject() 
	{
		severMappedMemeberFunc.clear(); 
		clientMappedMemeberFunc.clear();}
	;

	std::map < std::uintptr_t, std::vector<MemberFunc>>& GetMappedMemberFunc()
	{
		if(isStateServer)
			return severMappedMemeberFunc;
		else
			return clientMappedMemeberFunc;
	};

	void CallReplicatedFunction(class Object* pObj, std::string funcName);
	void CallReplicatedFunction(class Object* pObj, unsigned int idx);
	void SendRemoteProcedureCall(class Object* pObj, std::string funcName);
	void SendRemoteProcedureCall(class Object* pObj, unsigned int idx);
	void ReciveRemoteProcedureCall(std::queue<char>& packet, class LocalReplication* local);

private:
	std::map<std::uintptr_t, std::vector<MemberFunc>> severMappedMemeberFunc;
	std::map<std::uintptr_t, std::vector<MemberFunc>> clientMappedMemeberFunc;
};


/*
#define FUNC_BEGIN \
std::function<void()> funcRepliStart = [this]() \
{ \
auto mappedFunc = FunctionReplication::GetInstance()->mappedMemeberFunc; \
std::vector<MemberFunc> memVec;	\
auto id = std::uintptr_t(this);	\

//auto memvec = FunctionReplication::GetInstance()->memberFunc; \


#define FUNC_VOID(className, funcName) \
memVec.emplace_back(#funcName, std::bind(&className::funcName, static_cast<className*>(this))); \


#define FUNC_END  \
mappedFunc.insert({id, memVec}); };\

//memvec.emplace_back(#funcName, std::bind(&className::funcName, static_cast<className*>(this)));\
*/


//MemberFunc* funcName = new MemberFunc(#funcName, std::bind(&className::funcName, static_cast<className*>(this))); /\
//out.push_back(funcName);


#define FUNC_BEGIN \
virtual void FuncReplication() override \
{ \
auto& mappedFunc = FunctionReplication::GetInstance()->GetMappedMemberFunc(); \
auto id = this->objectID;	\
auto itor = mappedFunc.find(id);\
std::vector<MemberFunc> memVec;	\
unsigned int idx = 0;	\
bool inHerited = false;			\
if(itor != mappedFunc.end())	\
{\
inHerited = true;		\
memVec = itor->second;	\
idx = memVec.back().funcIndex + 1;	\
};\

#define FUNC_VOID(className, funcName) \
memVec.emplace_back(idx++, #funcName, std::bind(&className::funcName, static_cast<className*>(this))); \

#define FUNC_END  \
if(inHerited == false) \
{	\
	mappedFunc.insert({ id, memVec });\
}	\
else \
{	\
	auto& vec = itor->second;\
	vec.swap(memVec);\
}	\
};	\


//vec.insert(vec.end(), memVec.begin(), memVec.end());