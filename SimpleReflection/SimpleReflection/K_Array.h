#pragma once

/// <summary>
/// 리플렉션 될 클래스에 자료구조를 넣으려다보니,
/// 포인터에서 다시 값으로 변환하는 것이 불편해서 직접 만들게 됨
/// </summary>

template<typename T>
class K_Array
{
public:
	K_Array(T* pData, size_t size) : memSize(size), length(size)
	{
		data = new T[size];
		memcpy(data, pData, sizeof(T) * memSize);

		int a = 0;
	}

	K_Array() : memSize(1), length(0)
	{ 
		data = new T; 
	}

	void push_back(T&& element)
	{
		if (memSize <= length)
		{
			auto temp = data;
			data = new T[memSize * 2];

			memcpy(data, temp, sizeof(T) * memSize);
			memSize *= 2;
			delete[] temp;
		}

		data[length++] = element;
	}

	size_t size() const
	{
		return length;
	}

	T& at(size_t num)  const
	{
		return data[num];
	}

	T& operator[](size_t num) const 
	{
		return data[num];
	}

	T* GetData() const
	{
		return data;
	}

	bool IsEmpty() const { return (length <= 0); }

private:
	T* data;			
	size_t length;		// 현재 사이즈
	size_t memSize;		// 현재 배열의 크기

};

template<typename T>
static T* ReadArray(void* ptr)
{
	//return *reinterpret_cast<T**>(ptr);
	return **reinterpret_cast<T***>((std::byte*)ptr);
};

template<typename T>
static size_t ReadSize(void* ptr)
{
	return *reinterpret_cast<size_t*>((std::byte*)ptr + sizeof(T));
};
