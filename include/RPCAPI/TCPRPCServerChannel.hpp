#pragma once

#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

#include <string_view>

namespace RPCAPI
{
	// RPC channel over TCP for Server
	class TCPRPCServerChannel : public RPCAPI::RPCAPIChannel
	{
	private:
		netsocket::Socket m_listenSocket;
		netsocket::Socket m_connectedSocket;
	public:
		TCPRPCServerChannel();
	
		bool bindAndListen(std::string_view ipAddr, std::string_view port);
	
		bool acceptConnection();

		bool isConnected();
	
		// Implementation of RPCAPI::RPCAPIChannel interface
		virtual bool send(const u8* bytes, u32 length) override;
		virtual bool receive(u8* const bytes, u32 length) override;
	};
}
