#pragma once

#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

namespace RPCAPI
{
	class TCPRPCClientChannel : public RPCAPI::RPCAPIChannel
	{
	private:
		netsocket::Socket m_socket;
	public:
		TCPRPCClientChannel();
	
		bool connect(std::string_view ipAddr, std::string_view port);

		bool isConnected();
	
		virtual bool send(const u8* bytes, u32 length) override;
	
		virtual bool receive(u8* const bytes, u32 length) override;
	};
}
