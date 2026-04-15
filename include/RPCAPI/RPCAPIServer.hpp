#pragma once

#include <RPCAPI/SerDes.hpp> // for DeserializeArgs<>

#include <common/defines.hpp> // for com::unordered_map<>

#include <vector>
#include <string>
#include <functional>

namespace RPCAPI
{
	class RPCAPIChannel;
	
	class RPCAPIServer
	{
		using ByteHandler = std::function<void(const u8*, u32, std::vector<u8>&)>;
	private:
		com::unordered_map<std::string, ByteHandler> m_api;

	public:
		template<typename ReturnType, typename... ProcArgs, typename F>
		void define(std::string_view procName, const F&& handler)
		{
			ByteHandler byteHandler = [handler](const u8* bytes, u32 length, std::vector<u8>& retBytes)
			{
				auto argsTuple = DeserializeArgs<ProcArgs...>(bytes, length);
				auto returnedValue = std::apply(handler, argsTuple);
				Serialize<ReturnType> { } (retBytes, returnedValue);
			};
			m_api.insert({ std::string { procName }, std::move(byteHandler) });
		}
	
		void run(RPCAPIChannel& ch);
	};
}
