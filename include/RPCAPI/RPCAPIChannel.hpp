#pragma once

#include <common/defines.h>

namespace RPCAPI
{
	class RPCAPIChannel
	{
	public:
		virtual bool send(const u8* bytes, u32 length) = 0;
		virtual bool receive(u8* const bytes, u32 length) = 0;
	};
}
