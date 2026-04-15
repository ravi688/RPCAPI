from rpcapiclient import serdes
from rpcapiclient.types import u32, s32

class SecreteCode:
    def __init__(self, hoInt: u32 = 0, loInt: u32 = 0):
        self.hoInt = hoInt
        self.loInt = loInt
    def __str__(self):
    	return f"SecreteCode(hoInt={self.hoInt}, loInt={self.loInt})"

# Serialize into buffer
@serdes.register_serializer(SecreteCode)
def serialize_SecreteCode(buf : bytearray, value : SecreteCode):
	serdes.serialize(buf, u32(value.hoInt))
	serdes.serialize(buf, u32(value.loInt))

# Deserialize from buffer with offset (important for chaining)
@serdes.register_deserializer(SecreteCode)
def deserialize_SecreteCode(buf : bytes, offset: int):
	if len(buf) < offset + 8:
		raise ValueError("Not enough data to deserialize SecreteCode")
	v = SecreteCode()
	v.hoInt, offset = serdes.deserialize(buf, offset, u32)
	v.loInt, offset = serdes.deserialize(buf, offset, u32)
	return v, offset
