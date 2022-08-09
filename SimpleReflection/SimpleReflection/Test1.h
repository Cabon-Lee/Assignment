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
			std::cout << "오브젝트 이름 :" << std::endl;
			std::cin >> name;

			pObject = CLASS(name.c_str());
		}

		auto types = reflectionHelper.GetObjectMembers(pObject);
		std::cout << "오브젝트의 타입, 변수 이름은 이렇습니다." << std::endl;
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
			std::cout << "접근할 변수명을 입력하세요" << std::endl;
			std::cin >> name;

			pRet = reflectionHelper.AcccessToMember(pObject, name);
			if (pRet == nullptr) continue;	// nullptr이 나오면 다시 돌려보낸다

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

			std::cout << "변수 수정을 진행합니까?" << std::endl;
			std::cin >> command;

			if (strcmp(command.c_str(), "yes") == 0)
			{
				std::cout << "값을 입력하세요" << std::endl;
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
					// f를 붙이지 않으면 값이 변경되지 않습니다
					*reinterpret_cast<float*>(pRet->data) = std::stof(command);
					break;
				}
			}
		}
		delete pRet;
		std::cout << "최종 결과입니다" << std::endl;

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