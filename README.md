# RPCAPI
Tiny and simple remote procedure call library. Works on Linux and Windows both.

## Building and installing
```
build_master meson setup build --buildtype=release
build_master meson compile -C build
build_master meson install -C build
```

## Usage 
### Writing API Server code
**Full server code example**: https://github.com/ravi688/RPCAPI/blob/main/source/main.network.server.cpp
#### Define a communication channel for the server
The class should be derived from the `RPCAPI::RPCAPIChannel` interface class.
```cpp
//...
#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

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
		return true;
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
```
#### Define the API
```cpp
//...
#include <RPCAPI/RPCAPIServer.hpp>

//...
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
//...
```
#### Server API requests in a separate thread
```cpp
//...
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

//...
```
### Writing Client code
**Full client code example**: https://github.com/ravi688/RPCAPI/blob/main/source/main.network.client.cpp
#### Define a communication channel for the client
```cpp
//...
#include <RPCAPI/RPCAPIChannel.hpp>
#include <netsocket/netsocket.hpp>

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
```
#### Make API requests in a separate thread
```cpp
//...

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
```
## License
MIT
