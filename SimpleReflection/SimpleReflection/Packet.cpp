#include "Packet.h"

void Packet::AccessToServer(std::queue<char>& que, char& packetType, bool isGet)
{
	EnterCriticalSection(&g_cs);

	if (isGet)
	{
		if (toServer.empty() != true)
		{
			packetType = toServer.front();
			toServer.pop();

			if (packetType == PACKET_CLASS)
			{

				char tempSize[2];
				tempSize[0] = toServer.front();
				toServer.pop();
				tempSize[1] = toServer.front();
				toServer.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				if (packetSize <= toServer.size())
				{
					while (!toServer.empty())
					{
						que.push(toServer.front());
						toServer.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
				//char targetPacketSize = toServer.front() + sizeof(std::uintptr_t) + 1;
				// 패킷을 받아 올 때 원하는 크기 이상만큼 서버의 패킷이 차있어야 한다
				//if (targetPacketSize <= toServer.size())
				//{
				//	while (!toServer.empty())
				//	{
				//		que.push(toServer.front());
				//		toServer.pop();
				//
				//		// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
				//		if (que.size() == targetPacketSize)
				//		{
				//			break;
				//		}
				//	}
				//}
			}
			else if (packetType == PACKET_MEMBER)
			{
				char tempSize[2];
				tempSize[0] = toServer.front();
				toServer.pop();
				tempSize[1] = toServer.front();
				toServer.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				// if (packetSize <= toServer.size())
				// 패킷 사이즈 검사 시, 패킷의 사이즈를 담는 타입 만큼 빼줍니다
				if (packetSize <= toServer.size())
				{
					while (!toServer.empty())
					{
						que.push(toServer.front());
						toServer.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
			}
			else if (packetType == PACKET_RPC)
			{
				char tempSize[2];
				tempSize[0] = toServer.front();
				toServer.pop();
				tempSize[1] = toServer.front();
				toServer.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				if (packetSize <= toServer.size())
				{
					while (!toServer.empty())
					{
						que.push(toServer.front());
						toServer.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		packetType = 0;

		if (que.empty() != true)
		{
			while (!que.empty())
			{
				toServer.push(que.front());
				que.pop();
			}
		}
	}

	LeaveCriticalSection(&g_cs);
}

void Packet::AccessToClient(std::queue<char>& que, char& packetType, bool isGet)
{
	EnterCriticalSection(&g_cs);

	if (isGet)
	{
		if (toClient.empty() != true)
		{
			packetType = toClient.front();
			toClient.pop();

			if (packetType == PACKET_CLASS)
			{
				char tempSize[2];
				tempSize[0] = toClient.front();
				toClient.pop();
				tempSize[1] = toClient.front();
				toClient.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				if (packetSize <= toClient.size())
				{
					while (!toClient.empty())
					{
						que.push(toClient.front());
						toClient.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
			}
			else if (packetType == PACKET_MEMBER)
			{
				char tempSize[2];
				tempSize[0] = toClient.front();
				toClient.pop();
				tempSize[1] = toClient.front();
				toClient.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				if (packetSize <= toClient.size())
				{
					while (!toClient.empty())
					{
						que.push(toClient.front());
						toClient.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
			}
			else if (packetType == PACKET_RPC)
			{
				char tempSize[2];
				tempSize[0] = toClient.front();
				toClient.pop();
				tempSize[1] = toClient.front();
				toClient.pop();

				auto packetSize = *reinterpret_cast<unsigned short*>(tempSize);
				packetSize -= sizeof(unsigned short);
				if (packetSize <= toClient.size())
				{
					while (!toClient.empty())
					{
						que.push(toClient.front());
						toClient.pop();

						// 큐에 목표로 하는 크기만큼 패킷이 쌓였을 때 
						if (que.size() == packetSize)
						{
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		packetType = 0;

		if (que.empty() != true)
		{
			while (!que.empty())
			{
				toClient.push(que.front());
				que.pop();
			}
		}
	}

	LeaveCriticalSection(&g_cs);
}

void Packet::CanclationSever(size_t size)
{
	if (toServer.size() == size)
	{
		while (!toServer.empty())
			toServer.pop();
	}
	else if (toServer.size() > size)
	{
		std::queue<char> temp;
	
		unsigned int count = toServer.size() - size;
		while (count != 0)
		{
			temp.push(toServer.front());
			toServer.pop();
			count--;
		}

		toServer.swap(temp);
	}
}

void Packet::CanclationClinet(size_t size)
{
	if (toClient.size() == size)
	{
		while (!toClient.empty())
			toClient.pop();
	}
	else if (toClient.size() > size)
	{
		std::queue<char> temp;

		unsigned int count = toClient.size() - size;
		while (count != 0)
		{
			temp.push(toClient.front());
			toClient.pop();
			count--;
		}

		toClient.swap(temp);
	}
}

