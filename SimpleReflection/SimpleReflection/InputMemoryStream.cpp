#pragma once

#include <cstddef>
#include <vector>
#include <string>

class InputMemoryStream
{
public:
    InputMemoryStream(const void* data, std::size_t size) {};
    ~InputMemoryStream() {};

    void Read(void* dest, std::size_t size);

    template < typename Type >
    void Read(Type& out)
    {
        // 원시 자료형 여부 검사
        static_assert(
            std::is_arithmetic_v<Type> ||
            std::is_enum_v<Type>,
            "generic read only supports primitive data types");

        Read(&out, sizeof(Type));
    }

    template <>
    void Read<std::string>(std::string& out)
    {
        std::uint32_t length;
        Read(length);
        std::string temp;
        temp.resize(length);
        Read(temp.data(), length);
        out = std::move(temp);
    }

private:

    const void* data;
    std::size_t size;
    std::size_t index;
};
