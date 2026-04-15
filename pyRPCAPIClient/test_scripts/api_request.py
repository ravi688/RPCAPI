import rpcapiclient
from rpcapiclient.types import u32, to_u32
from secrete_code import SecreteCode

import socket
HOST = '127.0.0.1'
PORT = 2000

class ClientSocket(rpcapiclient.RPCAPIChannel):
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect(self, host: str, port: int) -> bool:
        try:
            self.sock.connect((host, port))
            return True
        except socket.error:
            return False

    def send(self, data: bytes, length: int) -> bool:
        try:
            total_sent = 0
            while total_sent < length:
                sent = self.sock.send(data[total_sent:])
                if sent == 0:
                    return False  # connection closed
                total_sent += sent
            return True
        except socket.error:
            return False

    def receive(self, buffer: bytearray, length: int) -> bool:
        try:
            total_received = 0
            while total_received < length:
                chunk = self.sock.recv(length - total_received)
                if not chunk:
                    return False  # connection closed
                buffer[total_received:total_received + len(chunk)] = chunk
                total_received += len(chunk)
            return True
        except socket.error:
            return False

    def close(self):
        self.sock.close()


def main():
	api_channel = ClientSocket()
	if not api_channel.connect(HOST, PORT):
		print(f'Failed to connect to the server at {HOST, PORT}')
		return
	api_client = rpcapiclient.RPCAPIClient(api_channel)
	print('calling get-kvm-port-count')
	count = api_client.call("get-kvm-port-count", u32)
	if not count is None:
		print(f'got count = {count}')
	else:
		print('get-kvm-port-count: something went wrong')
	print('calling try-kvm-port-reserve')
	isSuccess = api_client.call("try-kvm-port-reserve", bool, SecreteCode(4324, 4324234))
	if not isSuccess is None:
		print(f'got result = {isSuccess}')
	else:
		print('try-kvm-port-reserve: something went wrong')
	print('calling multiply-2-vector with [1, 2, 3, 4, 5]')
	doubled_vector = api_client.call("multiply-by-2-vector", list[u32], to_u32([1, 2, 3, 4, 5]))
	if not doubled_vector is None:
		print(f'doubled vector = {doubled_vector}')
	else:
		print('multiply-2-vector: something went wrong')
	api_channel.close()
	return

if __name__ == "__main__":
	main()