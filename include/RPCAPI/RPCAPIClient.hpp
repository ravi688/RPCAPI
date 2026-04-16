#pragma once

#include <RPCAPI/RPCAPIChannel.hpp>
#include <RPCAPI/SerDes.hpp>
#include <common/debug.h>

#include <string_view>
#include <vector>
#include <optional>
#include <stdexcept>

namespace RPCAPI
{
	class RPCAPIClient
	{
	private:
		RPCAPIChannel& m_ch;
	public:
		RPCAPIClient(RPCAPIChannel& ch) : m_ch(ch) { }
	
		template<typename ReturnType, typename... ProcArgs>
		std::optional<ReturnType> call(std::string_view procName, ProcArgs&&... args)
		{
			std::vector<u8> bytes;
			SerializeArgs<ProcArgs...>(bytes, std::forward<ProcArgs>(args)...);
			u32 procNameLen = static_cast<u32>(procName.size());
			if(!m_ch.send(reinterpret_cast<u8*>(&procNameLen), sizeof(u32)))
				return { };
			if(!m_ch.send(reinterpret_cast<const u8*>(procName.data()), procName.size()))
				return { };
			u32 argsLen = static_cast<u32>(bytes.size());
			if(!m_ch.send(reinterpret_cast<u8*>(&argsLen), sizeof(u32)))
				return { };
			if(!m_ch.send(bytes.data(), bytes.size()))
				return { };
			u32 retLen;
			if(!m_ch.receive(reinterpret_cast<u8*>(&retLen), sizeof(u32)))
				return { };
			std::vector<u8> receiveBytes(retLen);
			if(!m_ch.receive(receiveBytes.data(), receiveBytes.size()))
				return { };
			u32 offset = 0;
			try
			{
				auto res = Deserialize<ReturnType>{} (receiveBytes.data(), offset, receiveBytes.size());
				return res;
			}
			catch(const std::exception& e)
			{
				com_debug_log_error("Caught: %s", e.what());
				return { };
			}
			catch(...)
			{
				com_debug_log_error("Caught Unknown Exception");
				return { };
			}
		}
	};
}
