#pragma once

#include "Server.h"
#include "Client.h"
// 게임이 돌아가는데 필요한 기반들을 가지고 있는 클래스

class SimpleEngine
{
public:
	SimpleEngine();
	~SimpleEngine();
	
public:
	void main();
	// 엔진 위에서 서버가 돌아가야할 것 같은데
	Server server;
	Client client;

	void AdvanceQuery()
	{
		bool isAdvance = false;
		std::string command;
		while (!isAdvance)
		{
			std::cout << "다음으로" << std::endl;
			std::cin >> command;

			if (command.compare("y") == 0)
			{
				system("cls");
				break;
			}
		}
	}

	bool TerminateQuery()
	{
		bool isAdvance = false;
		std::string command;
		while (!isAdvance)
		{
			std::cout << "처음부터 돌까요? y/n" << std::endl;
			std::cin >> command;

			if (command.compare("y") == 0)
			{
				system("cls");
				return false;
			}
			else if (command.compare("n") == 0)
			{
				system("cls");
				return true;
			}
		}
	}

};

