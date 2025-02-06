#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"
#include "ArcCacheNode.h"


// 辅助函数，用于打印测试结果
void printTestResult(const std::string& testName, bool result) {
    std::cout << testName << ": " << (result ? "Passed" : "Failed") << std::endl;
}

int main() {
    // 测试默认构造函数
    FgCache::ArcNode<int, int> defaultNode;
    bool defaultConstructorTest = (defaultNode.getAccessCount() == 1);
    printTestResult("Default Constructor Test", defaultConstructorTest);

    // 测试带参数的构造函数
    int key = 1;
    int value = 100;
    FgCache::ArcNode<int, int> paramNode(key, value);
    bool paramConstructorTest = (paramNode.getKey() == key) && (paramNode.getValue() == value) && (paramNode.getAccessCount() == 1);
    printTestResult("Parameterized Constructor Test", paramConstructorTest);

    // 测试 setValue 方法
    FgCache::ArcNode<int, int> setValueNode(1, 100);
    int newValue = 200;
    setValueNode.setValue(newValue);
    bool setValueTest = (setValueNode.getValue() == newValue);
    printTestResult("setValue Method Test", setValueTest);

    // 测试 incrementAccessCount 方法
    FgCache::ArcNode<int, int> incrementNode(1, 100);
    incrementNode.incrementAccessCount();
    bool incrementTest = (incrementNode.getAccessCount() == 2);
    printTestResult("incrementAccessCount Method Test", incrementTest);

    return 0;
}