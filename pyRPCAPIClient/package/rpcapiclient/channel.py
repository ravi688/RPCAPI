from abc import ABC, abstractmethod

class RPCAPIChannel(ABC):
	@abstractmethod
	def send(self, data : bytes, len : int) -> bool:
		pass
	@abstractmethod
	def receive(self, data : bytearray, len : int) -> bool:
		pass
