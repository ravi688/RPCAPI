#include <RPCAPI/RPCAPIServer.hpp>
#include <RPCAPI/RPCAPIChannel.hpp>

#include <common/debug.h>

// For security reasons, we should always clamp the valid procedure name length to a fixed value, 
// so that a malicious client won't be able to send large proc name.
static constexpr u32 gMaxProcNameLen = 30;

namespace RPCAPI
{
	bool RPCAPIServer::serveRequest(RPCAPIChannel& ch)
	{
		u32 procNameLen;
		if(!ch.receive(reinterpret_cast<u8*>(&procNameLen), sizeof(u32)))
			return false;
		// procedure name length can't be greater than max allowed length
		if(procNameLen > gMaxProcNameLen)
		{
			com_debug_log_error("ProcName length (%u) exceeds the max allowed length", procNameLen);
			return false;
		}
		std::vector<char> procNameChars(procNameLen);
		if(!ch.receive(reinterpret_cast<u8*>(procNameChars.data()), procNameLen))
			return false;
		procNameChars.push_back('\0');
		std::string_view procName { procNameChars.data(), procNameChars.size() - 1 };
		auto it = m_api.find(procName);
		if(it == m_api.end())
		{
			com_debug_log_error("No procedure has been defined with name %s", procName.data());
			return false;
		}
		auto& bHandler = it->second;
		u32 argsLen;
		if(!ch.receive(reinterpret_cast<u8*>(&argsLen), sizeof(u32)))
			return false;
		std::vector<u8> bytes(argsLen);
		if(!ch.receive(bytes.data(), argsLen))
			return false;
		std::vector<u8> retBytes;
		try
		{
			bHandler(bytes.data(), static_cast<u32>(bytes.size()), retBytes);
		}
		catch(const std::exception& e)
		{
			com_debug_log_error("Caught: %s", e.what());
			return false;
		}
		catch(...)
		{
			com_debug_log_error("Caught unknown exception");
			return false;
		}
		u32 retBytesLen = static_cast<u32>(retBytes.size());
		if(!ch.send(reinterpret_cast<u8*>(&retBytesLen), sizeof(u32)))
			return false;
		if(!ch.send(retBytes.data(), retBytes.size()))
			return false;

		return true;
	}
}
