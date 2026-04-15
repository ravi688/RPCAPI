#include <RPCAPI/RPCAPIServer.hpp>
#include <RPCAPI/RPCAPIChannel.hpp>

#include <common/debug.h>

namespace RPCAPI
{
	bool RPCAPIServer::serveRequest(RPCAPIChannel& ch)
	{
		u32 procNameLen;
		if(!ch.receive(reinterpret_cast<u8*>(&procNameLen), sizeof(u32)))
			return false;
		std::vector<char> procNameChars(procNameLen);
		if(!ch.receive(reinterpret_cast<u8*>(procNameChars.data()), procNameLen))
			return false;
		procNameChars.push_back('\0');
		std::string_view procName { procNameChars.data(), procNameChars.size() - 1 };
		// com_debug_log_info("server: received proc name: %s", procName.data());
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
		bHandler(bytes.data(), static_cast<u32>(bytes.size()), retBytes);
		if(!ch.send(retBytes.data(), retBytes.size()))
			return false;

		return true;
	}
}
