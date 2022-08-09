#pragma once

#include <iostream>
#include "ReflectionHelper.h"
static ReflectionHelper reflectionHelper;

class TestScenario1
{
public:
	TestScenario1()
	{
		std::string name;

		Object* pObject = nullptr;
		while (pObject == nullptr)
		{
			std::cout << "������Ʈ �̸� :" << std::endl;
			std::cin >> name;

			pObject = CLASS(name.c_str());
		}

		auto types = reflectionHelper.GetObjectMembers(pObject);
		std::cout << "������Ʈ�� Ÿ��, ���� �̸��� �̷����ϴ�." << std::endl;
		for (const auto& nowVar : types)
		{
			std::cout << nowVar.GetTypeName() << " " << nowVar.GetName() << std::endl;
		}

		std::string command;
		Returner* pRet = nullptr;
		while (strcmp(command.c_str(), "no") != 0 && pRet == nullptr)
		{
			if (pRet != nullptr) delete pRet;

			name.clear();
			std::cout << "������ �������� �Է��ϼ���" << std::endl;
			std::cin >> name;

			pRet = reflectionHelper.AcccessToMember(pObject, name);
			if (pRet == nullptr) continue;	// nullptr�� ������ �ٽ� ����������

			switch (pRet->myType)
			{
			case EType::STRING:
				std::cout << reinterpret_cast<std::string*>(pRet->data)->c_str() << std::endl;
				break;
			case EType::INT:
				std::cout << *reinterpret_cast<int*>(pRet->data) << std::endl;
				break;
			case EType::FLOAT:
				std::cout << *reinterpret_cast<float*> (pRet->data) << "f" << std::endl;
				break;
			}

			std::cout << "���� ������ �����մϱ�?" << std::endl;
			std::cin >> command;

			if (strcmp(command.c_str(), "yes") == 0)
			{
				std::cout << "���� �Է��ϼ���" << std::endl;
				std::cin >> command;

				switch (pRet->myType)
				{
				case EType::STRING:
					*reinterpret_cast<std::string*>(pRet->data) = command;
					break;
				case EType::INT:
					*reinterpret_cast<int*>(pRet->data) = std::stoi(command);
					break;
				case EType::FLOAT:
					// f�� ������ ������ ���� ������� �ʽ��ϴ�
					*reinterpret_cast<float*>(pRet->data) = std::stof(command);
					break;
				}
			}
		}
		delete pRet;
		std::cout << "���� ����Դϴ�" << std::endl;

		pRet = reflectionHelper.AcccessToMember(pObject, name);
		switch (pRet->myType)
		{
		case EType::STRING:
			std::cout << reinterpret_cast<std::string*>(pRet->data)->c_str() << std::endl;
			break;
		case EType::INT:
			std::cout << *reinterpret_cast<int*>(pRet->data) << std::endl;
			break;
		case EType::FLOAT:
			std::cout << *reinterpret_cast<float*>(pRet->data) << "f" << std::endl;
			break;
		}
	}

};