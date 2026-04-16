from .channel import RPCAPIChannel
from .serdes import serialize, deserialize
from .types import u32

class RPCAPIClient:
	def __init__(self, channel : RPCAPIChannel):
		self.channel = channel

	def call(self, proc_name : str, ret_type, *args):

		# Send the procedure name to be invoked on the API server
		proc_name_bytes = bytearray()
		# String serialization automatically prefixes the string length before the string content
		serialize(proc_name_bytes, proc_name)
		if not self.channel.send(proc_name_bytes, len(proc_name_bytes)):
			return None

		# Send the arguments, also make sure the serialized byte sequence is prefixed with the sequence length
		args_bytes = bytearray()
		for arg in args:
			serialize(args_bytes, arg)
		args_len_bytes = bytearray()
		serialize(args_len_bytes, u32(len(args_bytes)))
		if not self.channel.send(args_len_bytes, len(args_len_bytes)):
			return None
		if not self.channel.send(args_bytes, len(args_bytes)):
			return None

		# Receive the returned byte count
		ret_bytes_len = bytearray(4)
		if not self.channel.receive(ret_bytes_len, len(ret_bytes_len)):
			return None
		try:
			ret_len, _ = deserialize(ret_bytes_len, 0, u32)
		except:
			print('Exception caught, Failed to deserialize byte count for return value')
			return None
		# Receive the returned bytes
		ret_bytes = bytearray(ret_len)
		if not self.channel.receive(ret_bytes, ret_len):
			return None
		# Deserialize the returned bytes to reconstructed the returned object
		offset : int = 0
		try:
			ret_value, offset = deserialize(ret_bytes, offset, ret_type)
		except:
			print('Exception caught, Failed to deserialize return value')
			return None
		return ret_value
