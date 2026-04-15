#include <RPCAPI/RPCAPIServer.hpp>
#include <RPCAPI/RPCAPIClient.hpp>
#include <RPCAPI/MockRPCChannel.hpp>

int main(int argc, const char** argv)
{
	using SecreteCode = RPCAPI::SecreteCode;
	RPCAPI::RPCAPIServer apiServer;
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

	RPCAPI::BidirectionalChannel ch;
	std::thread serverThread([&ch, &apiServer]()
	{
		auto mkCh = ch.getEndPoint1();
		apiServer.run(mkCh);
	});

	auto mkCh = ch.getEndPoint2();
	RPCAPI::RPCAPIClient apiClient(mkCh);
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

	clientThread.join();
	serverThread.join();

	return 0;
}
