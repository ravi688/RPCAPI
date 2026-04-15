#pragma once

#include <RPCAPI/RPCAPIChannel.hpp>
#include <RPCAPI/SerDes.hpp>

#include <string_view>
#include <vector>
#include <optional>

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
			// com_debug_log_info("client: Sending call to %s", procName.data());
			u32 procNameLen = static_cast<u32>(procName.size());
			m_ch.send(reinterpret_cast<u8*>(&procNameLen), sizeof(u32));
			m_ch.send(reinterpret_cast<const u8*>(procName.data()), procName.size());
			u32 argsLen = static_cast<u32>(bytes.size());
			m_ch.send(reinterpret_cast<u8*>(&argsLen), sizeof(u32));
			m_ch.send(bytes.data(), bytes.size());
			// com_debug_log_info("client: Call to %s is sent", procName.data());
			// com_debug_log_info("client: waiting for the return value");
			u32 retLen;
			m_ch.receive(reinterpret_cast<u8*>(&retLen), sizeof(u32));
			std::vector<u8> receiveBytes(retLen);
			m_ch.receive(receiveBytes.data(), receiveBytes.size());
			u32 offset = 0;
			auto res = Deserialize<ReturnType>{} (receiveBytes.data(), offset);
			return res;
		}
	};
}
