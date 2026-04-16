#include <RPCAPI/TCPRPCServerChannel.hpp>

namespace RPCAPI
{
	TCPRPCServerChannel::TCPRPCServerChannel() : m_listenSocket(netsocket::SocketType::Stream, 
								netsocket::IPAddressFamily::IPv4,
								netsocket::IPProtocol::TCP)
	{
	}
	
	bool TCPRPCServerChannel::bindAndListen(std::string_view ipAddr, std::string_view port)
	{
		auto result = m_listenSocket.bind(ipAddr, port);
		if(result != netsocket::Result::Success)
			return false;
		result = m_listenSocket.listen();
		if(result != netsocket::Result::Success)
			return false;
		return true;
	}
	
	bool TCPRPCServerChannel::acceptConnection()
	{
		auto incomingCon = m_listenSocket.accept();
		if(incomingCon.has_value())
			m_connectedSocket = std::move(incomingCon.value());
		else
			return false;
		return true;
	}

	bool TCPRPCServerChannel::isConnected()
	{
		return m_connectedSocket.isConnected();
	}
	
	bool TCPRPCServerChannel::send(const u8* bytes, u32 length)
	{
		return m_connectedSocket.send(bytes, length) == netsocket::Result::Success;
	}
	
	bool TCPRPCServerChannel::receive(u8* const bytes, u32 length)
	{
		return m_connectedSocket.receive(bytes, length) == netsocket::Result::Success;
	}
}
