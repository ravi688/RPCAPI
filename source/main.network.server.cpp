#include <RPCAPI/RPCAPIServer.hpp>
#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

#include <thread>

class ServerSocket : public RPCAPI::RPCAPIChannel
{
private:
	netsocket::Socket m_listenSocket;
	netsocket::Socket m_connectedSocket;
public:
	ServerSocket() : m_listenSocket(netsocket::SocketType::Stream, 
								netsocket::IPAddressFamily::IPv4,
								netsocket::IPProtocol::TCP)
	{
	}

	bool bindAndListen()
	{
		auto result = m_listenSocket.bind("127.0.0.1", "2000");
		if(result != netsocket::Result::Success)
			return false;
		result = m_listenSocket.listen();
		if(result != netsocket::Result::Success)
			return false;
	}

	bool acceptConnection()
	{
		auto incomingCon = m_listenSocket.accept();
		if(incomingCon.has_value())
			m_connectedSocket = std::move(incomingCon.value());
		else
			return false;
		return true;
	}

	virtual bool send(const u8* bytes, u32 length) override
	{
		return m_connectedSocket.send(bytes, length) == netsocket::Result::Success;
	}

	virtual bool receive(u8* const bytes, u32 length) override
	{
		return m_connectedSocket.receive(bytes, length) == netsocket::Result::Success;
	}
};

using SecreteCode = RPCAPI::SecreteCode;

int main(int argc, const char** argv)
{
	// Create a RPCAPIServer instance
	RPCAPI::RPCAPIServer apiServer;

	// Define the API handlers
	apiServer.define<u32>(std::string_view { "get-kvm-port-count" }, []() -> u32
	{
		com_debug_log_info("server: get-kvm-port-count: sending 8");
		return 8;
	});
	apiServer.define<bool, SecreteCode>("try-kvm-port-reserve", [](const SecreteCode& secrete)
	{
		com_debug_log_info("server: try-kvm-port-reserve(%u:%u), sending true", secrete[0], secrete[1]);
		return true;
	});
	apiServer.define<std::vector<u32>, std::vector<u32>>("multiply-by-2-vector", [](const std::vector<u32>& v)
	{
		std::vector<u32> dv;
		for(const auto& el : v)
		{
			dv.push_back(2 * el);
		}
		return dv;
	});


	// Create a communication channel
	ServerSocket socket;
	if(!socket.bindAndListen())
	{
		com_debug_log_error("Failed to bind to 127.0.0.1:2000");
		return -1;
	}
	if(!socket.acceptConnection())
	{
		com_debug_log_error("Failed to accept connection");
		return -1;
	}

	// Run the API request server threead
	std::atomic<bool> isServerStop = false;
	std::thread serverThread([&socket, &apiServer, &isServerStop]()
	{
		while(!isServerStop)
		{
			apiServer.serveRequest(socket);
			std::this_thread::sleep_for(std::chrono::duration<float, std::ratio<1, 1>>(1));
		}
	});

	// Close the server after 10 seconds
	com_debug_log_info("Stopping the server in 10 seconds");
	std::this_thread::sleep_for(std::chrono::duration<float, std::ratio<1, 1>>(10));
	isServerStop = true;
	serverThread.join();

	return 0;
}
