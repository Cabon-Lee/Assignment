#pragma once

#include <iostream>
#include "ReflectionHelper.h"
#include "SimpleEngine.h"
#include "K_Array.h"
#include "ObjectManager.h"
#include "Server.h"
#include "Packet.h"
#include "PacketProcessor.h"

#include "Function.h"

#include <tuple>
#include <array>
#include <utility>

void f(int i, std::string const& s) {
	std::cout << "f(" << i << ", " << s << ")\n";
}
void f(int i, double d, std::string const& s) {
	std::cout << "f(" << i << ", " << d << ", " << s << ")\n";
}
void f(char c, int i, double d, std::string const& s) {
	std::cout << "f(" << c << ", " << i << ", " << d << ", " << s << ")\n";
}
void f(int i, int j, int k) {
	std::cout << "f(" << i << ", " << j << ", " << k << ")\n";
}


// ----------------------------------------------------------------------------
// The actual function expanding the tuple:
template <typename Tuple, std::size_t... I>
void process(Tuple const& tuple, std::index_sequence<I...>)
{
	f(std::get<I>(tuple)...);
}

// 매개변수가 들어있는 튜플을 입력한다

// The interface to call. Sadly, it needs to dispatch to another function
// to deduce the sequence of indices created from std::make_index_sequence<N>
template <typename Tuple>
void process(Tuple const& tuple)
{
	process(tuple, std::make_index_sequence<std::tuple_size<Tuple>::value>());
}

template <typename... Ts>
class A {
public:
	static inline auto funcs = std::make_tuple(std::function<Ts()>()...);
};

auto call = [](auto&&...funcs) {
	(funcs(), ...);
};




int main()
{

	/*
	Custom* a = new Custom;

	auto mem = FunctionReplication::GetInstance()->GetMappedMemberFunc();
	FunctionReplication::GetInstance()->CallReplicatedFunction(a, 0);

	A<int, float, float> l;

	std::get<0>(l.funcs) = []() { std::cout << "Calling int()" <<   std::endl; return 1; };
	std::get<1>(l.funcs) = []() { std::cout << "Calling float()" << std::endl; return 1.f; };
	std::get<2>(l.funcs) = []() { std::cout << "Calling char()" <<  std::endl; return '1'; };

	std::apply(call, l.funcs);
	*/



	//f.DoSomething("first");

	//Function c;
	//c.func();

	//f.func(&f);
	
	SimpleEngine engine;
	engine.main();

	return 0;
}