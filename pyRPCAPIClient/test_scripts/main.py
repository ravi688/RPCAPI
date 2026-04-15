from rpcapiclient import serdes
from rpcapiclient.types import u32, s32, to_u32
from secrete_code import *

def main():
	buf = bytearray()
	serdes.serialize(buf, u32(123))
	serdes.serialize(buf, s32(-1))

	value1, offset = serdes.deserialize(buf, 0, u32)
	value2, offset = serdes.deserialize(buf, offset, s32)
	print('deserialized value1 = ' + str(value1))
	print('deserialized value2 = ' + str(value2))

	code = SecreteCode(2434, 3424)
	print('serialized secrete code = ' + str(code))
	serdes.serialize(buf, code)

	des_code, offset = serdes.deserialize(buf, offset, SecreteCode)
	print('deserialized secrete code = ' + str(des_code))

	my_list : list[u32] = to_u32([ 2, 3, 4, 5 ])
	print('serialized list[u32]: ' + str(my_list))
	serdes.serialize(buf, my_list)

	des_my_list, offset = serdes.deserialize(buf, offset, list[u32])
	print('deserialized list[u32]: ' + str(des_my_list))

if __name__ == "__main__":
	main()