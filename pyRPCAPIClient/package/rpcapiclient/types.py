class u32(int): pass
class s32(int): pass

def to_u32(value):
	if type(value) is list:
		converted_list = []
		for item in value:
			converted_list.append(to_u32(item))
		return converted_list
	elif type(value) is int:
		return u32(value)
	raise TypeError(f"Currently this type {type(value)} is not handled")
