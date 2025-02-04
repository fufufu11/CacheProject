#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"

// ���Թ��ߺ���
void printTestResult(bool passed, const std::string& testName) {
    if (passed) {
        std::cout << "[PASS] " << testName << std::endl;
    }
    else {
        std::cout << "[FAIL] " << testName << std::endl;
    }
}

// ��������
void TestBasicFunction() {
    LfuCache<int, std::string> cache(3);
    bool passed = true;

    // ����Ͷ�ȡ
    cache.put(1, "A");
    std::string value;
    passed &= cache.get(1, value) && (value == "A");

    printTestResult(passed, "TestBasicFunction");
}

void TestEvictionPolicy() {
    LfuCache<int, std::string> cache(2);
    bool passed = true;

    cache.put(1, "A");
    cache.put(2, "B");
    cache.put(3, "C"); // ������̭

    std::string val;
    passed &= !cache.get(1, val); // 1����̭
    passed &= cache.get(3, val) && (val == "C"); // 3����

    printTestResult(passed, "TestEvictionPolicy");
}
void TestFrequencyIncrease() {
    LfuCache<int, int> cache(3);
    bool passed = true;

    cache.put(1, 100);
    int val;
    cache.get(1, val);
    cache.get(1, val);

    passed &= (cache.nodeMap_[1]->freq == 3); // Ƶ��ӦΪ3

    printTestResult(passed, "TestFrequencyIncrease");
}
void TestEdgeCases() {
    bool passed = true;

    // ����Ϊ0�Ļ���
    LfuCache<int, std::string> cache0(0);
    cache0.put(1, "A"); // ��Ӧ����
    passed &= (cache0.nodeMap_.size() == 0);

    // ��̭�߼�
    LfuCache<int, std::string> cache1(1);
    cache1.put(1, "A");
    cache1.put(2, "B"); // ��̭1
    std::string val;
    passed &= !cache1.get(1, val);

    printTestResult(passed, "TestEdgeCases");
}

int main()
{
    //TestBasicFunction();
    //TestEvictionPolicy();
    //TestFrequencyIncrease();
    TestEdgeCases();
    return 0;
}