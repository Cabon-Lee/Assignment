#pragma once

#include "Server.h"
#include "Client.h"
// ������ ���ư��µ� �ʿ��� ��ݵ��� ������ �ִ� Ŭ����

class SimpleEngine
{
public:
	SimpleEngine();
	~SimpleEngine();
	
public:
	void main();
	// ���� ������ ������ ���ư����� �� ������
	Server server;
	Client client;

	void AdvanceQuery()
	{
		bool isAdvance = false;
		std::string command;
		while (!isAdvance)
		{
			std::cout << "��������" << std::endl;
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
			std::cout << "ó������ �����? y/n" << std::endl;
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

