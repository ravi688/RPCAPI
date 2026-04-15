#pragma once

#include <common/defines.h>
#include <array>
#include <vector>
#include <cstring> // for std::memcpy

namespace RPCAPI
{
	using SecreteCode = std::array<u32, 2>;

	template<typename T>
	struct Deserialize
	{
		T operator()(const u8* data, u32& offset);
	};

	template<>
	struct Deserialize<u32>
	{
		u32 operator()(const u8* data, u32& offset)
		{
			u32 value;
		    std::memcpy(&value, data + offset, sizeof(u32));
		    offset += sizeof(u32);
		    return value;
		}
	};

	template<>
	struct Deserialize<bool>
	{
		bool operator()(const u8* data, u32& offset)
		{
			u8 _bool;
			std::memcpy(&_bool, data + offset, 1);
			offset += 1;
			return (_bool == 1) ? true : false;
		}	
	};

	template<>
	struct Deserialize<SecreteCode>
	{
		SecreteCode operator()(const u8* data, u32& offset)
		{
			SecreteCode code;
			std::memcpy(&code[0], data + offset, sizeof(code));
			offset += sizeof(code);
			return code;
		}
	};

	template<typename T>
	struct Deserialize<std::vector<T>>
	{
		std::vector<T> operator()(const u8* data, u32& offset)
		{
			u32 count = Deserialize<u32>{}(data, offset);
			std::vector<T> v;
			v.reserve(count);
			for(u32 i = 0; i < count; ++i)
				v.push_back(Deserialize<T>{}(data, offset));
			return v;
		}
	};

	template<typename... Args, std::size_t... I>
	auto DeserializeArgsImpl(const u8* data, u32 length, std::index_sequence<I...>)
	{
		u32 offset = 0;
		return std::tuple<Args...>{
			Deserialize<Args>{} (data, offset)...
		};
	}

	template<typename... Args>
	auto DeserializeArgs(const u8* data, u32 length)
	{
		return DeserializeArgsImpl<Args...>(data, length, std::index_sequence_for<Args...>{});
	}

	template<typename T>
	struct Serialize
	{
		void operator()(std::vector<u8>& bytes, T value);
	};

	template<>
	struct Serialize<u32>
	{
		void operator()(std::vector<u8>& bytes, u32 value)
		{
			bytes.resize(bytes.size() + sizeof(u32));
			auto* ptr = &bytes[bytes.size() - sizeof(u32)];
			std::memcpy(ptr, &value, sizeof(u32));
		}
	};

	template<>
	struct Serialize<bool>
	{
		void operator()(std::vector<u8>& bytes, bool value)
		{
			bytes.push_back(1);
			u8 _bool = (value ? 1 : 0);
			auto* ptr = &bytes[bytes.size() - 1];
			std::memcpy(ptr, &_bool, 1);
		}
	};

	template<>
	struct Serialize<SecreteCode>
	{
		void operator()(std::vector<u8>& bytes, SecreteCode code)
		{
			bytes.resize(bytes.size() + sizeof(SecreteCode));
			auto* ptr = &bytes[bytes.size() - sizeof(SecreteCode)];
			std::memcpy(ptr, &code, sizeof(SecreteCode));
		}
	};

	template<typename T>
	struct Serialize<std::vector<T>>
	{
		void operator()(std::vector<u8>& bytes, const std::vector<T>& v)
		{
			Serialize<u32> {}(bytes, v.size());
			for(const auto& el : v)
				Serialize<T> {}(bytes, el);
		}
	};

	template<typename... Args>
	void SerializeArgs(std::vector<u8>& bytes, Args&&... args)
	{
		(Serialize<Args>{} (bytes, std::forward<Args>(args)), ...);
	}
}
