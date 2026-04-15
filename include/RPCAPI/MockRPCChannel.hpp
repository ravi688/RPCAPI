#pragma once

#include <RPCAPI/RPCAPIChannel.hpp>

#include <deque>
#include <mutex>
#include <condition_variable>

namespace RPCAPI
{
	class UnidirectionalChannel
	{
	private:
		std::deque<u8> m_queue;
		std::mutex m_mtx;
		std::condition_variable m_cv;
	public:
		bool send(const u8* bytes, u32 length)
		{
			std::unique_lock<std::mutex> ulock(m_mtx);
			for(u32 i = 0; i < length; ++i)
				m_queue.push_back(bytes[i]);
			ulock.unlock();
			m_cv.notify_one();
			return true;
		}
		bool receive(u8* const bytes, u32 length)
		{
			if(length == 0) return false;
			std::unique_lock<std::mutex> ulock(m_mtx);
			m_cv.wait(ulock, [this, length]() { return m_queue.size() >= length; });
			for(u32 i = 0; i < length; ++i)
			{
				bytes[i] = m_queue.front();
				m_queue.pop_front();
			}
			ulock.unlock();
			m_cv.notify_one();
			return true;
		}
	};
	
	class BidirectionalChannel;

	class ChannelEndPoint : public RPCAPIChannel
	{
		friend class BidirectionalChannel;
	private:
		UnidirectionalChannel& m_sendChannel;
		UnidirectionalChannel& m_receiveChannel;
		ChannelEndPoint(UnidirectionalChannel& sendCh, UnidirectionalChannel& receiveCh) : m_sendChannel(sendCh), m_receiveChannel(receiveCh) { }
	public:
		virtual bool send(const u8* bytes, u32 length)
		{
			return m_sendChannel.send(bytes, length);
		}
		virtual bool receive(u8* const bytes, u32 length)
		{
			return m_receiveChannel.receive(bytes, length);
		}
	};
	
	class BidirectionalChannel
	{
	private:
		UnidirectionalChannel m_sendChannel;
		UnidirectionalChannel m_receiveChannel;
	public:
		ChannelEndPoint getEndPoint1()
		{
			return { m_sendChannel, m_receiveChannel };
		}
		ChannelEndPoint getEndPoint2()
		{
			return { m_receiveChannel, m_sendChannel };
		}
	};
}
