#pragma once

#include <common/defines.h>
#include <array>
#include <vector>
#include <cstring> // for std::memcpy
#include <stdexcept>

namespace RPCAPI
{
	using SecreteCode = std::array<u32, 2>;

	template<typename T>
	struct Deserialize
	{
		T operator()(const u8* data, u32& offset, u32 length);
	};

	class IncrementOffsetOnExit
	{
	private:
		u32& m_offset;
		u32 m_size;
	public:
		IncrementOffsetOnExit(u32& offset, u32 size, u32 length) : m_offset(offset), m_size(size)
		{
			if((offset + size) > length)
				throw std::runtime_error("Unsufficient bytes to deserialize");
		}
		~IncrementOffsetOnExit()
		{
			m_offset += m_size;
		}
	};

	template<>
	struct Deserialize<u32>
	{
		u32 operator()(const u8* data, u32& offset, u32 length)
		{
			IncrementOffsetOnExit _(offset, sizeof(u32), length);
			u32 value;
		    std::memcpy(&value, data + offset, sizeof(u32));
		    return value;
		}
	};

	template<>
	struct Deserialize<bool>
	{
		bool operator()(const u8* data, u32& offset, u32 length)
		{
			IncrementOffsetOnExit _(offset, sizeof(u8), length);
			u8 _bool;
			std::memcpy(&_bool, data + offset, 1);
			return (_bool == 1) ? true : false;
		}
	};

	template<>
	struct Deserialize<SecreteCode>
	{
		SecreteCode operator()(const u8* data, u32& offset, u32 length)
		{
			IncrementOffsetOnExit _(offset, sizeof(SecreteCode), length);
			SecreteCode code;
			std::memcpy(&code[0], data + offset, sizeof(code));
			return code;
		}
	};

	template<typename T>
	struct Deserialize<std::vector<T>>
	{
		std::vector<T> operator()(const u8* data, u32& offset, u32 length)
		{
			u32 count = Deserialize<u32>{}(data, offset, length);
			std::vector<T> v;
			v.reserve(count);
			for(u32 i = 0; i < count; ++i)
				v.push_back(Deserialize<T>{}(data, offset, length));
			return v;
		}
	};

	template<>
	struct Deserialize<std::string>
	{
		std::string operator()(const u8* data, u32& offset, u32 length)
		{
			u32 size = Deserialize<u32> {}(data, offset, length);
			IncrementOffsetOnExit _(offset, sizeof(u8) * size, length);
			auto str = std::string { std::string_view { reinterpret_cast<const char*>(data + offset), size } };
			return str;
		}
	};

	template<typename... Args, std::size_t... I>
	auto DeserializeArgsImpl(const u8* data, u32 length, std::index_sequence<I...>)
	{
		u32 offset = 0;
		return std::tuple<Args...>{
			Deserialize<Args>{} (data, offset, length)...
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

	template<>
	struct Serialize<std::string>
	{
		void operator()(std::vector<u8>& bytes, const std::string& s)
		{
			Serialize<u32> {}(bytes, s.size());
			bytes.resize(bytes.size() + s.size());
			auto* dstPtr = &bytes[bytes.size() - s.size()];
			auto* srcPtr = s.data();
			std::memcpy(dstPtr, srcPtr, s.size());
		}
	};	

	template<typename... Args>
	void SerializeArgs(std::vector<u8>& bytes, const Args&... args)
	{
		(Serialize<Args>{} (bytes, args), ...);
	}
}
