from .types import u32, s32

_serializers = {}
_deserializers = {}

# Decorator for registering a serializer function for a given type
def register_serializer(typ):
    def wrapper(fn):
        _serializers[typ] = fn
        return fn
    return wrapper

# Decorator for registering a deserializer function for a given type
def register_deserializer(typ):
    def wrapper(fn):
        _deserializers[typ] = fn
        return fn
    return wrapper


# Searches for appropriate serializer based on the type inference and calls it
def serialize(buf: bytearray, value):
    typ = type(value)
    if isinstance(value, list):
        buf += struct.pack("<I", u32(len(value)))
        for item in value:
            serialize(buf, item)
        return

    if typ in _serializers:
        return _serializers[typ](buf, value)

    raise TypeError(f"No serializer for {typ}")

# Searches for appropriate deserilizer based on the type specified and calls it
def deserialize(buf: bytes, offset: int, typ):
    from typing import get_origin, get_args

    origin = get_origin(typ)
    if origin is list:
        inner_type = get_args(typ)[0]
        count, offset = deserialize(buf, offset, u32)
        result = []
        for _ in range(count):
            item, offset = deserialize(buf, offset, inner_type)
            result.append(item)
        return result, offset

    if typ in _deserializers:
        return _deserializers[typ](buf, offset)

    raise TypeError(f"No deserializer for {typ}")


# Serializers and Deserializers

import struct

# u32
# ------------------------
# Serializer
@register_serializer(u32)
def serialize_u32(buf: bytearray, value: u32):
	if not (0 <= value <= 2**32):
		raise ValueError("u32 out of range")
	buf += struct.pack("<I", value)
# Deserializer
@register_deserializer(u32)
def deserialize_u32(buf: bytes, offset: int):
    value = struct.unpack_from("<I", buf, offset)[0]
    return u32(value), offset + 4
# ------------------------

# s32
# ------------------------
# Serializer
@register_serializer(s32)
def serialize_s32(buf : bytearray, value : s32):
	if not (-2**31 <= value < 2**31):
		raise ValueError("s32 out of range")
	buf += struct.pack("<i", value)
# Deserializer
@register_deserializer(s32)
def deserialize_s32(buf : bytes, offset : int):
	value = struct.unpack_from("<i", buf, offset)[0]
	return s32(value), offset + 4
# -----------------------

