#pragma once

#include <vector>
#include <cstddef>
#include <typeinfo>
#include <map>
#include <functional>

#include "rtti.h"
#include "Function.h"
#include "Singleton.h"
#include "Replication.h"


struct InitialStringData
{
	unsigned int index;
	std::string varName;
	std::string initialString;
};

class OutputMemoryStream : public Singleton<OutputMemoryStream>
{
public:

	std::pair<const std::byte*, size_t> GetBufferData() const
	{
		return { buffer.data(), head };
	}

	const std::byte* GetBufferPointer() const
	{
		return buffer.data();
	}

	size_t GetBufferSize() const
	{
		return head;
	}

	void Write(const void* data, size_t offset, size_t size)
	{
		Reserve(offset + size);
		std::memcpy(buffer.data() + offset, data, size);
		head = offset + size;
	}

	template < typename Type >
	void DoWrite(const Type& value, size_t offset)
	{
		Write(&value, offset, sizeof(value));
	}

	void StringWrite(std::string& val, size_t offset)
	{
		Reserve(head + sizeof(std::string) + offset);
		std::memcpy(buffer.data() + head, val.data(), val.size());
		head = head + sizeof(std::string) + offset;
	}

	void WritePtr(const void* data, size_t offset, size_t size)
	{
		Reserve(offset + size);
		long long ptr = -1;	// nullptr ���, ���� ��ȿ���� Ȯ���ϱ� ���� �����ѹ�
		std::memcpy(buffer.data() + offset, &ptr, size);
		head = offset + size;
	}

	void WriteFunc(const void* data, size_t offset)
	{
		size_t funcSize;
#ifdef _x64
		funcSize = 8;
#else
		funcSize = 4;
#endif
		Reserve(offset + funcSize);
		std::memcpy(buffer.data() + head, data, funcSize);
		head = head + funcSize + offset;
	}

	void Reserve(std::size_t size)
	{
		buffer.resize(size);
	}

	size_t head = 0;
	std::vector<std::byte> buffer;
	std::map<std::string, std::vector<MemberVariable>> refleTypeInfo;	// Ŭ������ ��� ���� �������� ����
	std::map<std::string, std::vector<std::byte>> refleInitData;		// Ŭ������ �ʱ�ȭ ������ ����
	std::map<std::string, std::vector<InitialStringData>> reflInitStringData;		// Ŭ������ �ʱ�ȭ �� ��Ʈ�� ������ ����

	bool IsDerivedClass(const DataType& dataType, const void* src, std::string& name);

public:
	std::vector<std::string> GetVariableNames(const std::string&) const;
	std::vector<MemberVariable> GetVariables(const std::string&) const;
	std::map<std::string, std::vector<MemberVariable>>& GetReflectedTypeInfo();
};

template <typename _T>
static void Serialize(OutputMemoryStream& stream, const DataType& dataType, const void* src)
{
	stream.head = 0;
	stream.buffer.clear();

	// ��� �޾��� ��츦 ���� �н�
	std::string parentName("class ");
	std::vector<MemberVariable> memVars;
	std::vector<InitialStringData> stringMemeber;

	// ����� �޴��� Ȯ���ϰ�, ���ο��� �̸��� �����´�
	bool isDerived = stream.IsDerivedClass(dataType, src, parentName);
	if(isDerived == true)
	{

		for (const auto& memberVariable : stream.refleTypeInfo.at(parentName.c_str()))
		{
			std::byte* p = (std::byte*)src + memberVariable.GetOffset();
			size_t offsetSize = memberVariable.GetOffset();
			switch (memberVariable.GetType())
			{
			case PrimitiveType::INT:
				stream.DoWrite(*(std::int32_t*)p, offsetSize);
				break;
			case PrimitiveType::FLOAT:
				stream.DoWrite(*(float*)p, offsetSize);
				break;
			case PrimitiveType::STRING:
			{
				stream.StringWrite(*(std::string*)p, offsetSize);
				InitialStringData initData;
				initData.index = memberVariable.GetIndex();
				initData.varName = memberVariable.GetName();
				initData.initialString = *(std::string*)p;
				stringMemeber.push_back(initData);
				break;
			}
			case PrimitiveType::LIST:
			{
				stream.WritePtr((void*)p, offsetSize, sizeof(void*));
				break;
			}
			case PrimitiveType::VECTOR:
			{
				stream.WritePtr((void*)p, offsetSize, sizeof(void*));
				break;
			}
			case PrimitiveType::PTR:
				stream.DoWrite((void*)p, offsetSize);
				break;
			case PrimitiveType::FUNC:
				stream.WriteFunc(p, offsetSize);
				break;
			}

			// �θ��� ��� ���� ������ �����Ѵ�
			memVars.push_back(memberVariable);
		}
	}

	for (const auto& memberVariable : dataType.GetMemberVariables())
	{
		std::byte* p = (std::byte*)src + memberVariable.GetOffset();
		size_t offsetSize = memberVariable.GetOffset();
		switch (memberVariable.GetType())
		{
		case PrimitiveType::INT:
			stream.DoWrite(*(std::int32_t*)p, offsetSize);
			break;
		case PrimitiveType::FLOAT:
			stream.DoWrite(*(float*)p, offsetSize);
			break;
		case PrimitiveType::STRING:
		{
			stream.StringWrite(*(std::string*)p, offsetSize);
			InitialStringData initData;
			initData.index = memberVariable.GetIndex();
			initData.varName = memberVariable.GetName();
			initData.initialString = *(std::string*)p;
			stringMemeber.push_back(initData);
			break;
		}
		case PrimitiveType::LIST:
		{
			stream.WritePtr((void*)p, offsetSize, sizeof(void*));
			break;
		}
		case PrimitiveType::VECTOR:
		{
			stream.WritePtr((void*)p, offsetSize, sizeof(void*));
			break;
		}
		case PrimitiveType::PTR:
			// p�� ���� ���� ������ ������ Ŭ���� ���ο����� ��ġ���Դϴ�.
			// ���� ���⿡�� ����Ǵ� initial ptr ���� ���� �ǹ̰� �����ϴ�.
			stream.WritePtr((void*)p, offsetSize, sizeof(void*));
			break;
		case PrimitiveType::FUNC:
			stream.WriteFunc(p, offsetSize);
			break;
		}
		
		if (isDerived == true)
		{
			// �������� �о�ִ´�
			memVars.push_back(memberVariable);
		}
	}

	std::string className = typeid(_T).name();

	if (isDerived)
	{
		stream.refleTypeInfo.insert({ className, memVars });
		Replication::GetInstance()->EvaluateAndStoreVariable(className, memVars);
	}
	else
	{
		stream.refleTypeInfo.insert({ className, dataType.GetMemberVariables() });
		Replication::GetInstance()->EvaluateAndStoreVariable(className, dataType.GetMemberVariables());
	}

	if (!stringMemeber.empty())
	{
		stream.reflInitStringData.insert({ className, stringMemeber });
	}

	stream.refleInitData.insert({ className, stream.buffer });
}
