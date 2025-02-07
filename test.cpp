#include <iostream>
#include <cassert>
#include <thread>
#include "LRU.h"
#include "LFU.h"
#include "ArcCacheNode.h"
#include "ArcLruPart.h"

using namespace FgCache;

// ���Բ���ͻ�ȡ����
void testPutAndGet()
{
    ArcLruPart<std::string, std::string> cache(3, 3);

    // �����ֵ��
    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");

    std::string value;
    bool shouldTransform;

    // ��ȡ��ֵ��
    if (cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Put/Get: key1 found, value: " << value << ", shouldTransform: " << shouldTransform << std::endl;
    }
    else
    {
        std::cout << "Test Put/Get: key1 not found (unexpected)" << std::endl;
    }

    // ���Ի�����������
    cache.put("key4", "value4");

    // ��֤ key2 ������
    if (!cache.get("key2", value, shouldTransform))
    {
        std::cout << "Test Put/Get: key2 evicted successfully" << std::endl;
    }
}
// ������������黺�湦��
void testEvictionAndGhostCache()
{
    ArcLruPart<std::string, std::string> cache(3, 3);

    cache.put("key1", "value1");
    cache.put("key2", "value2");
    cache.put("key3", "value3");

    cache.put("key4", "value4"); // ���� key1 �����黺��
    cache.put("key5", "value5"); // ���� key2 �����黺��

    std::string value;
    bool shouldTransform;

    // ��� key1 �Ƿ������黺����
    if (cache.checkGhost("key1"))
    {
        std::cout << "Test Eviction/GhostCache: key1 found in ghost cache" << std::endl;
    }

    // ��֤ key1 ������
    if (!cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Eviction/GhostCache: key1 successfully evicted from main cache" << std::endl;
    }

    // ���²��� key1���������黺�������
    //   3 4 5
    //  1 2
    cache.put("key1", "value1");

    // �������黺������ key2 �� key3
    // ��֤ key2 �Ƿ���Ȼ�����黺����
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



// ����������������
void testCapacityAdjustment()
{
    ArcLruPart<std::string, std::string> cache(2, 3);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // ��������
    cache.increaseCapacity();
    std::cout << "Test Capacity Adjustment: Capacity increased to 3" << std::endl;

    // �����¼�ֵ�ԣ����ᴥ������
    cache.put("key3", "value3");

    std::string value;
    bool shouldTransform;

    if (cache.get("key1", value, shouldTransform))
    {
        std::cout << "Test Capacity Adjustment: key1 found after capacity increase" << std::endl;
    }

    // ��������
    cache.decreaseCapacity();
    std::cout << "Test Capacity Adjustment: Capacity decreased back to 2" << std::endl;

    // �����¼�ֵ�ԣ���������
    cache.put("key4", "value4");

    if (!cache.get("key2", value, shouldTransform))
    {
        std::cout << "Test Capacity Adjustment: key2 evicted after capacity decrease" << std::endl;
    }
}
// ���Է��ʼ�����ת���ż�
void testAccessCountAndThreshold()
{
    ArcLruPart<std::string, std::string> cache(3, 2);

    cache.put("key1", "value1");
    cache.put("key2", "value2");

    // ���� key1 ����
    std::string value;
    bool shouldTransform;
    cache.get("key1", value, shouldTransform);
    cache.get("key1", value, shouldTransform);

    if (shouldTransform)
    {
        std::cout << "Test AccessCount/Threshold: key1 shouldTransform returns true" << std::endl;
    }

    // ���� key2 ��ֵ
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