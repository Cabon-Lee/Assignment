#pragma once

template<typename T>
class K_Node
{
public:
	K_Node(T pData) : data(pData), pNext(nullptr), pPrev(nullptr) {}

	K_Node* pNext;
	K_Node* pPrev;
	T data;
};

template<typename T>
class K_List
{
private:
	K_Node<T>* pHead;
	K_Node<T>* pTail;

public:
	K_List(K_Node<T>* head) : pHead(head), pTail(nullptr) {}
	K_List() : pHead(nullptr), pTail(nullptr) {}

	/*
	void push_back(T&& pData)
	{
		if (pHead == nullptr)
		{
			pHead = new K_Node<T>(pData);
		}
		else
		{
			K_Node<T>* pNewNode = new K_Node<T>(pData);
			if (pTail == nullptr)
			{
				pTail = pNewNode;
				pHead->pNext = pTail;
				pTail->pPrev = pHead;
			}
			else
			{
				pTail->pNext = pNewNode;
				pNewNode->pPrev = pTail;
				pTail = pNewNode;
			}
		}
	}
	*/

	void push_back(T pData)
	{
		if (pHead == nullptr)
		{
			pHead = new K_Node<T>(pData);
		}
		else
		{
			K_Node<T>* pNewNode = new K_Node<T>(pData);
			if (pTail == nullptr)
			{
				pTail = pNewNode;
				pHead->pNext = pTail;
				pTail->pPrev = pHead;
			}
			else
			{
				pTail->pNext = pNewNode;
				pNewNode->pPrev = pTail;
				pTail = pNewNode;
			}
		}
	}



	void Erase(K_Node<T>* pTarget)
	{
		if (pTarget->pPrev == nullptr)	// ����
		{
			if (pTarget->pNext == nullptr)	// ���鼭 �����̶�, �ϳ� ���� ���´�
			{
				pHead = nullptr; pTail = nullptr;
			}
			pTarget->pNext->pPrev = nullptr;
			pHead = pTarget->pNext;
		}
		else // ��尡 �ƴϴ�
		{
			if (pTarget->pNext == nullptr)	// �����̴�
			{
				pTail = pTarget->pPrev;
			}
			pTarget->pNext->pPrev = pTarget->pPrev;
			pTarget->pPrev->pNext = pTarget->pNext;

		}

		delete pTarget;
		pTarget = nullptr;
	}

	bool IsEmpty() { return (pHead == nullptr && pTail == nullptr); }
	K_Node<T>* GetHead() const { return pHead; }
	K_Node<T>* GetTail() const { return pTail; }
};