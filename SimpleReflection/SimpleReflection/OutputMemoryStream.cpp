#include "OutputMemoryStream.h"
#include "RuntimeClass.h"
#include "sample.h"

bool OutputMemoryStream::IsDerivedClass(const DataType& dataType, const void* src, std::string& name)
{
	// ���� ���� ��ü�� ����� ���� �Ļ� ��ü���� ���� Ȯ���մϴ�.
	Object* pObj = static_cast<Object*>(const_cast<void*>(src));
	if (sizeof(Object) < dataType.GetMemberVariables().front().GetOffset())
	{
		// �̷����ϸ� ��ӹ��� Ŭ������ �̸��� ����Ű�� �ּҸ� �˰ԵǴ� �� ����
		void* p = (std::byte*)src + dataType.GetMemberVariables().front().GetOffset() - sizeof(const char*);
		auto pName = (const char**)(p);

		name += *pName;
		return true;
	}

	return false;
}

std::vector<std::string> OutputMemoryStream::GetVariableNames(const std::string& className) const
{
	auto typeInfo = refleTypeInfo.at(className);
	std::vector<std::string> varName(typeInfo.size());

	for (int i = 0; i < typeInfo.size(); i++)
	{
		varName[i] = typeInfo[i].GetName();
	}

	return varName;
}

std::vector<MemberVariable> OutputMemoryStream::GetVariables(const std::string& className) const
{
	return refleTypeInfo.at(className);
}

std::map<std::string, std::vector<MemberVariable>>& OutputMemoryStream::GetReflectedTypeInfo()
{
	return refleTypeInfo;
}

