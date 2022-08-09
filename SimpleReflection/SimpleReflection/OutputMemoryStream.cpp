#include "OutputMemoryStream.h"
#include "RuntimeClass.h"
#include "sample.h"

bool OutputMemoryStream::IsDerivedClass(const DataType& dataType, const void* src, std::string& name)
{
	// 현재 들어온 객체가 상속을 받은 파생 객체인지 먼저 확인합니다.
	Object* pObj = static_cast<Object*>(const_cast<void*>(src));
	if (sizeof(Object) < dataType.GetMemberVariables().front().GetOffset())
	{
		// 이렇게하면 상속받은 클래스의 이름을 가리키는 주소를 알게되는 건 맞음
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

