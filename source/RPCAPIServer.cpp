#include <RPCAPI/RPCAPIServer.hpp>
#include <RPCAPI/RPCAPIChannel.hpp>

#include <common/debug.h>

#include <thread>

namespace RPCAPI
{
	void RPCAPIServer::run(RPCAPIChannel& ch)
	{
		// com_debug_log_info("Running the RPCAPIServer");
		u32 requestServed = 0;
		while(true)
		{
			u32 procNameLen;
			ch.receive(reinterpret_cast<u8*>(&procNameLen), sizeof(u32));
			std::vector<char> procNameChars(procNameLen);
			ch.receive(reinterpret_cast<u8*>(procNameChars.data()), procNameLen);
			procNameChars.push_back('\0');
			std::string_view procName { procNameChars.data(), procNameChars.size() - 1 };
			// com_debug_log_info("server: received proc name: %s", procName.data());
			auto it = m_api.find(procName);
			if(it == m_api.end())
			{
				com_debug_log_error("No procedure has been defined with name %s", procName.data());
				return;
			}
			auto& bHandler = it->second;
			u32 argsLen;
			ch.receive(reinterpret_cast<u8*>(&argsLen), sizeof(u32));
			std::vector<u8> bytes(argsLen);
			ch.receive(bytes.data(), argsLen);
			std::vector<u8> retBytes;
			bHandler(bytes.data(), static_cast<u32>(bytes.size()), retBytes);
			ch.send(retBytes.data(), retBytes.size());
			std::this_thread::sleep_for(std::chrono::duration<float, std::ratio<1, 1>>(1));
			++requestServed;
			if(requestServed >= 3)
				break;
		}
		// com_debug_log_info("Exiting the RPCAPIServer");
	}
}
