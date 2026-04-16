#include <RPCAPI/TCPRPCClientChannel.hpp>

namespace RPCAPI
{
	TCPRPCClientChannel::TCPRPCClientChannel() : m_socket(netsocket::SocketType::Stream, 
								netsocket::IPAddressFamily::IPv4,
								netsocket::IPProtocol::TCP)
	{
	}
	
	bool TCPRPCClientChannel::connect(std::string_view ipAddr, std::string_view port)
	{
		auto result = m_socket.connect(ipAddr, port);
		return result == netsocket::Result::Success;
	}
	
	bool TCPRPCClientChannel::send(const u8* bytes, u32 length)
	{
		return m_socket.send(bytes, length) == netsocket::Result::Success;
	}
	
	bool TCPRPCClientChannel::receive(u8* const bytes, u32 length)
	{
		return m_socket.receive(bytes, length) == netsocket::Result::Success;
	}
}
