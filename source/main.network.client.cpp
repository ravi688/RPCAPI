#include <RPCAPI/RPCAPIClient.hpp>
#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

#include <thread>

class ClientSocket : public RPCAPI::RPCAPIChannel
{
private:
	netsocket::Socket m_socket;
public:
	ClientSocket() : m_socket(netsocket::SocketType::Stream, 
								netsocket::IPAddressFamily::IPv4,
								netsocket::IPProtocol::TCP)
	{
	}

	bool connect()
	{
		auto result = m_socket.connect("127.0.0.1", "2000");
		return result == netsocket::Result::Success;
	}

	virtual bool send(const u8* bytes, u32 length) override
	{
		return m_socket.send(bytes, length) == netsocket::Result::Success;
	}

	virtual bool receive(u8* const bytes, u32 length) override
	{
		return m_socket.receive(bytes, length) == netsocket::Result::Success;
	}
};

using SecreteCode = RPCAPI::SecreteCode;

int main(int argc, const char** argv)
{
	// Create and Establish a communication channel
	ClientSocket socket;
	if(!socket.connect())
	{
		com_debug_log_error("Failed to connect to the server");
		return -1;
	}

	// Create RPCAPIClient instance
	RPCAPI::RPCAPIClient apiClient(socket);

	// Run a client thread which issues API requests and receives response.
	std::thread clientThread([&apiClient]()
	{
		std::optional<u32> value = apiClient.call<u32>("get-kvm-port-count");
		if(value)
			com_debug_log_info("client: get-kvm-port-count, received %u", *value);
		else
			com_debug_log_error("client: get-kvm-port-count, something went wrong");

		std::optional<bool> isSuccess = apiClient.call<bool, SecreteCode>("try-kvm-port-reserve", { 43243u, 4324234u });
		if(isSuccess)
			com_debug_log_info("client: try-kvm-port-reserve, received %s", isSuccess ? "true" : "false");
		else
			com_debug_log_error("client: try-kvm-port-reserve, something went wrong");

		std::optional<std::vector<u32>> v = apiClient.call<std::vector<u32>, std::vector<u32>>("multiply-by-2-vector", { 1, 2, 3, 4, 5 });
		if(v)
		{
			com_debug_log_info("client: multiply-by-2-vector, received vector:");
			const auto& _v = *v;
			for(const auto& el : _v)
				std::cout << el << " ";
		}
		else
			com_debug_log_error("client: multiply-by-2-vector, something went wrong");
	});

	// Exit the client thread.
	clientThread.join();

	return 0;
}
