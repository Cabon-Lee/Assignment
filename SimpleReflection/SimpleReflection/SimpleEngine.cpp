#include "SimpleEngine.h"

SimpleEngine::SimpleEngine()
{

}

SimpleEngine::~SimpleEngine()
{

}

void SimpleEngine::main()
{
	bool isEnd = false;

	std::cout << "Ŭ���̾�Ʈ �ʱ�ȭ." << std::endl;
	std::cout << "���带 �����մϴ�." << std::endl;

	client.Start();

	while (!isEnd)
	{
		std::cout << "���� ����" << std::endl;
		server.Start();
		AdvanceQuery();

		std::cout << "���� ��Ŷ ó��" << std::endl;
		server.PacketProcess();
		AdvanceQuery();

		std::cout << "���� ƽ" << std::endl;
		server.Tick();
		AdvanceQuery();

		std::cout << "���� ������ �÷���" << std::endl;
		server.GarbageCollection();
		AdvanceQuery();

		std::cout << "Ŭ�� ����" << std::endl;
		client.Start();
		AdvanceQuery();

		std::cout << "Ŭ�� ��Ŷ ó��" << std::endl;
		client.PacketProcess();
		AdvanceQuery();

		std::cout << "Ŭ�� ƽ" << std::endl;
		client.Tick();
		AdvanceQuery();

		std::cout << "Ŭ�� ����" << std::endl;
		client.Render();
		AdvanceQuery();

		std::cout << "Ŭ�� ������ �÷���" << std::endl;
		client.GarbageCollection();
		AdvanceQuery();


		isEnd = TerminateQuery();
	}


}
