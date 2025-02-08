#pragma once
#include <memory>
#include"CachePolicy.h"
#include "ArcCacheNode.h"
#include "ArcLfuPart.h"
#include "ArcLruPart.h"

namespace FgCache
{
template<typename Key,typename Value>
class ArcCache : public FgCachePolicy<Key,Value>
{
public:
	explicit ArcCache(size_t capacity = 10,size_t transformThrehold = 2)
		:capacity_(capacity)
		,transformThreshold_(transformThrehold)
		,lruPart_(std::make_unique<ArcLruPart<Key,Value>>(capacity,transformThrehold))
		, lfuPart_(std::make_unique<ArcLfuPart<Key, Value>>(capacity, transformThrehold))
	{}
	~ArcCache() override = default;
	void put(Key key, Value value) override
	{
		bool inGhost = checkGhostCaches(key);
		if (!inGhost)
		{
			if (lruPart_->put(key, value))
			{
				lfuPart_->put(key, value);
			}
		}
		else
		{
			lruPart_->put(key, value);
		}
	}
	bool get(Key key, Value& value) override
	{
		checkGhostCaches(key);
		bool shouldTransform = false;
		if (lruPart_->get(key, value, shouldTransform))
		{
			if (shouldTransform)
			{
				lfuPart_->put(key, value);
			}
			return true;
		}
		return lfuPart_->get(key, value);
	}
	
	Value get(Key key) override
	{
		Value value{};
		get(key, value);
		return value;
	}
public:
	bool checkGhostCaches(Key key)
	{
		bool inGhost = false;
		if (lruPart_->checkGhost(key))
		{
			if (lfuPart_->decreaseCapacity())
			{
				lruPart_->increaseCapacity();
			}
			inGhost = true;
		}
		else if (lfuPart_->checkGhost(key))
		{
			if (lruPart_->decreaseCapacity())
			{
				lfuPart_->increaseCapacity();
			}
			inGhost = true;
		}
		return inGhost;
	}
private:
	size_t capacity_;
	size_t transformThreshold_;
	std::unique_ptr<ArcLruPart<Key, Value>> lruPart_;
	std::unique_ptr<ArcLfuPart<Key, Value>> lfuPart_;
};
}