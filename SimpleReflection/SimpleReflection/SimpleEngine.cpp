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

	std::cout << "클라이언트 초기화." << std::endl;
	std::cout << "월드를 생성합니다." << std::endl;

	client.Start();

	while (!isEnd)
	{
		std::cout << "서버 시작" << std::endl;
		server.Start();
		AdvanceQuery();

		std::cout << "서버 패킷 처리" << std::endl;
		server.PacketProcess();
		AdvanceQuery();

		std::cout << "서버 틱" << std::endl;
		server.Tick();
		AdvanceQuery();

		std::cout << "서버 가비지 컬렉션" << std::endl;
		server.GarbageCollection();
		AdvanceQuery();

		std::cout << "클라 시작" << std::endl;
		client.Start();
		AdvanceQuery();

		std::cout << "클라 패킷 처리" << std::endl;
		client.PacketProcess();
		AdvanceQuery();

		std::cout << "클라 틱" << std::endl;
		client.Tick();
		AdvanceQuery();

		std::cout << "클라 렌더" << std::endl;
		client.Render();
		AdvanceQuery();

		std::cout << "클라 가비지 컬렉션" << std::endl;
		client.GarbageCollection();
		AdvanceQuery();


		isEnd = TerminateQuery();
	}


}
