#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"
#include "ArcCacheNode.h"


// �������������ڴ�ӡ���Խ��
void printTestResult(const std::string& testName, bool result) {
    std::cout << testName << ": " << (result ? "Passed" : "Failed") << std::endl;
}

int main() {
    // ����Ĭ�Ϲ��캯��
    FgCache::ArcNode<int, int> defaultNode;
    bool defaultConstructorTest = (defaultNode.getAccessCount() == 1);
    printTestResult("Default Constructor Test", defaultConstructorTest);

    // ���Դ������Ĺ��캯��
    int key = 1;
    int value = 100;
    FgCache::ArcNode<int, int> paramNode(key, value);
    bool paramConstructorTest = (paramNode.getKey() == key) && (paramNode.getValue() == value) && (paramNode.getAccessCount() == 1);
    printTestResult("Parameterized Constructor Test", paramConstructorTest);

    // ���� setValue ����
    FgCache::ArcNode<int, int> setValueNode(1, 100);
    int newValue = 200;
    setValueNode.setValue(newValue);
    bool setValueTest = (setValueNode.getValue() == newValue);
    printTestResult("setValue Method Test", setValueTest);

    // ���� incrementAccessCount ����
    FgCache::ArcNode<int, int> incrementNode(1, 100);
    incrementNode.incrementAccessCount();
    bool incrementTest = (incrementNode.getAccessCount() == 2);
    printTestResult("incrementAccessCount Method Test", incrementTest);

    return 0;
}