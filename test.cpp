#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"


// ���Թ��ߺ���
template<typename T>
void print_test_result(const char* test_name, T expected, T actual) {
    if (expected == actual) {
        std::cout << "[PASS] " << test_name << "\n";
    }
    else {
        std::cout << "[FAIL] " << test_name
            << " (Expected: " << expected
            << ", Actual: " << actual << ")\n";
    }
}
// ��������
void TestBasicFunction() {
    HashLfuCache<int, std::string> cache(100, 4);

    // ���������ѯ
    cache.put(1, "A");
    std::string value;
    bool found = cache.get(1, value);
    print_test_result("BasicPutGet", true, found);
    print_test_result("ValueCorrect", std::string("A"), value);
}
void TestShardingDistribution() {
    HashLfuCache<int, int> cache(100, 2);
    int same_hash_count = 0;

    // ���Թ�ϣ�ֲ�
    for (int i = 0; i < 100; ++i) {
        size_t h1 = cache.Hash(i) % 2;
        size_t h2 = cache.Hash(i + 1) % 2;
        if (h1 == h2) same_hash_count++;
    }

    print_test_result("ShardingDistribution", false, same_hash_count == 100);
}
void TestEvictionLogic() {
    HashLfuCache<int, int> cache(3, 1); // ����Ƭ����3

    // ��仺��
    cache.put(1, 100);
    cache.put(2, 200);
    cache.put(3, 300);
    cache.put(4, 400); // Ӧ������̭

    int val;
    bool found = cache.get(4, val);
    std::cout << val << std::endl;
    print_test_result("EvictionHappened", false, found);
}
void TestConcurrency() {
    HashLfuCache<int, int> cache(1000, 8);
    constexpr int kThreads = 8;
    std::vector<std::thread> threads;

    // ��������
    for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < 1000; ++j) {
                int key = i * 1000 + j;
                cache.put(key, key);
                int value;
                cache.get(key, value);
            }
            });
    }

    for (auto& t : threads) t.join();
    std::cout << "[PASS] ConcurrencyStressTest\n";
}
void TestEdgeCases() {
    // ����Ϊ0�Ļ���
    HashLfuCache<int, std::string> cache0(0, 4);
    cache0.put(1, "A");
    std::string val;
    bool found = cache0.get(1, val);
    print_test_result("ZeroCapacity", false, found);

    // ����Ƭ����
    HashLfuCache<int, int> cache1(10, 1);
    for (int i = 0; i < 15; ++i) cache1.put(i, i);
    int tmp;
    print_test_result("SingleShardEvict", false, cache1.get(0, tmp));
}

int main() {
    //TestBasicFunction();
    //TestShardingDistribution();
    //TestEvictionLogic();
    //TestConcurrency();
    TestEdgeCases();
    return 0;
}