#include <iostream>
#include <cassert>
#include <thread>
#include "LRU.h"
#include "LFU.h"
#include "ArcCacheNode.h"
#include "ArcLruPart.h"

using namespace FgCache;

// 测试插入和获取功能
void testPutAndGet()
{
    ArcLruPart<std::string, std::string> cache(3, 3);

    // 插入键值对
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");

    std::string value;
    bool shouldTransform;

    // 获取键值对
    if (cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Put/Get: key1 found, value: " << value << ", shouldTransform: " << shouldTransform << std::endl;
    }
    else
    {
        std::cout << "Test Put/Get: key1 not found (unexpected)" << std::endl;
    }

    // 测试缓存容量限制
    cache.put("key4", "value4");

    // 验证 key2 被驱逐
    if (!cache.get("key2", value, shouldTransform))
    {
        std::cout << "Test Put/Get: key2 evicted successfully" << std::endl;
    }
}
// 测试驱逐和幽灵缓存功能
void testEvictionAndGhostCache()
{
    ArcLruPart<std::string, std::string> cache(3, 3);

    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");

    cache.put("key4", "value4"); // 驱逐 key1 到幽灵缓存
    cache.put("key5", "value5"); // 驱逐 key2 到幽灵缓存

    std::string value;
    bool shouldTransform;

    // 检查 key1 是否在幽灵缓存中
    if (cache.checkGhost("key1"))
    {
        std::cout << "Test Eviction/GhostCache: key1 found in ghost cache" << std::endl;
    }

    // 验证 key1 的驱逐
    if (!cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Eviction/GhostCache: key1 successfully evicted from main cache" << std::endl;
    }

    // 重新插入 key1，触发幽灵缓存的驱逐
    //   3 4 5
    //  1 2
    cache.put("key1", "value1");

    // 现在幽灵缓存中有 key2 和 key3
    // 验证 key2 是否仍然在幽灵缓存中
    if (cache.checkGhost("key2"))
    {
        std::cout << "Test Eviction/GhostCache: key2 found in ghost cache (still present)" << std::endl;
    }
    else
    {
        std::cout << "Test Eviction/GhostCache: key2 erroneously evicted from ghost cache" << std::endl;
    }

    if (cache.checkGhost("key1"))
    {
        std::cout << "Test Eviction/GhostCache: key1 found in ghost cache (still present)" << std::endl;
    }
    else
    {
        std::cout << "Test Eviction/GhostCache: key1 erroneously evicted from ghost cache" << std::endl;
    }
    if (cache.checkGhost("key3"))
    {
        std::cout << "Test Eviction/GhostCache: key3 found in ghost cache (still present)" << std::endl;
    }
    else
    {
        std::cout << "Test Eviction/GhostCache: key3 erroneously evicted from ghost cache" << std::endl;
    }
}



// 测试容量调整功能
void testCapacityAdjustment()
{
    ArcLruPart<std::string, std::string> cache(2, 3);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // 增加容量
    cache.increaseCapacity();
    std::cout << "Test Capacity Adjustment: Capacity increased to 3" << std::endl;

    // 插入新键值对，不会触发驱逐
    cache.put("key3", "value3");

    std::string value;
    bool shouldTransform;

    if (cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Capacity Adjustment: key1 found after capacity increase" << std::endl;
    }

    // 减少容量
    cache.decreaseCapacity();
    std::cout << "Test Capacity Adjustment: Capacity decreased back to 2" << std::endl;

    // 插入新键值对，触发驱逐
    cache.put("key4", "value4");

    if (!cache.get("key2", value, shouldTransform))
    {
        std::cout << "Test Capacity Adjustment: key2 evicted after capacity decrease" << std::endl;
    }
}
// 测试访问计数和转换门槛
void testAccessCountAndThreshold()
{
    ArcLruPart<std::string, std::string> cache(3, 2);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // 访问 key1 两次
    std::string value;
    bool shouldTransform;
    cache.get("key1", value, shouldTransform);
    cache.get("key1", value, shouldTransform);

    if (shouldTransform)
    {
        std::cout << "Test AccessCount/Threshold: key1 shouldTransform returns true" << std::endl;
    }

    // 更新 key2 的值
    cache.put("key2", "newValue2");

    cache.get("key2", value, shouldTransform);
    std::cout << "Test AccessCount/Threshold: key2's new value retrieved: " << value << std::endl;
}
int main() {
    //testPutAndGet();
   // testEvictionAndGhostCache();
   // testCapacityAdjustment();
    testAccessCountAndThreshold();
    return 0;
}