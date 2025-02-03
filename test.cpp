#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"

// 测试正常移除中间节点
void TestRemoveMiddleNode() {
    FreqList<int, std::string> freqList(1);

    // 创建三个测试节点并添加到链表
    auto node1 = std::make_shared<FreqList<int, std::string>::Node>(1, "A");
    auto node2 = std::make_shared<FreqList<int, std::string>::Node>(2, "B");
    auto node3 = std::make_shared<FreqList<int, std::string>::Node>(3, "C");
    freqList.addNode(node1);
    freqList.addNode(node2);
    freqList.addNode(node3);

    // 验证初始连接关系
    assert(node1->next == node2);
    assert(node2->pre == node1);
    assert(node2->next == node3);
    assert(node3->pre == node2);

    // 移除中间节点
    freqList.removeNode(node2);

    // 验证新连接关系
    assert(node1->next == node3);
    assert(node3->pre == node1);

    // 验证被移除节点的指针置空
    assert(node2->pre == nullptr);
    assert(node2->next == nullptr);

    printf("TestRemoveMiddleNode passed.\n");
}
// 测试移除头节点（非法操作）
void TestRemoveHeadSentinel() {
    FreqList<int, int> freqList(1);

    // 尝试移除头哨兵节点
    freqList.removeNode(freqList.head_);

    // 验证哨兵节点仍然连接
    assert(freqList.head_->next == freqList.tail_);
    assert(freqList.tail_->pre == freqList.head_);

    printf("TestRemoveHeadSentinel passed.\n");
}
// 测试移除尾节点（非法操作）
void TestRemoveTailSentinel() {
    FreqList<int, double> freqList(2);

    // 尝试移除尾哨兵节点
    freqList.removeNode(freqList.tail_);

    // 验证链表结构完整
    assert(freqList.head_->next == freqList.tail_);
    assert(freqList.tail_->pre == freqList.head_);

    printf("TestRemoveTailSentinel passed.\n");
}
// 测试移除不存在于链表的节点
void TestRemoveOrphanNode() {
    FreqList<int, char> freqList(3);

    // 创建两个独立节点
    auto nodeA = std::make_shared<FreqList<int, char>::Node>(1, 'X');
    auto nodeB = std::make_shared<FreqList<int, char>::Node>(2, 'Y');

    // 将nodeA加入链表
    freqList.addNode(nodeA);

    // 尝试移除未连接的nodeB
    freqList.removeNode(nodeB);  // 应跳过移除

    // 验证链表结构不变
    assert(freqList.head_->next == nodeA);
    assert(nodeA->pre == freqList.head_);

    printf("TestRemoveOrphanNode passed.\n");
}
void TestGetFirstNode()
{
    FreqList<int, char> freqList(3);
    auto nodeA = std::make_shared<FreqList<int, char>::Node>(1, 'A');
    freqList.addNode(nodeA);
    auto nodeB = freqList.getFirstNode();
    assert(nodeB == nodeA);
    std::cout << "Pass" << std::endl;
}
// 测试空链表状态
void TestEmptyList() {
    FreqList<int, std::string> freqList(3);

    // 验证空链表返回尾哨兵
    assert(freqList.getFirstNode() == freqList.tail_);
    printf("TestEmptyList passed.\n");
}
// 测试单节点场景
void TestSingleNode() {
    FreqList<int, double> freqList(2);

    auto node = std::make_shared<FreqList<int, double>::Node>(1, 3.14);
    freqList.addNode(node);

    // 验证获取首个节点
    assert(freqList.getFirstNode() == node);
    printf("TestSingleNode passed.\n");
}
// 测试多节点场景
void TestMultipleNodes() {
    FreqList<std::string, int> freqList(5);

    auto node1 = std::make_shared<FreqList<std::string, int>::Node>("first", 100);
    auto node2 = std::make_shared<FreqList<std::string, int>::Node>("second", 200);
    auto node3 = std::make_shared<FreqList<std::string, int>::Node>("third", 300);

    freqList.addNode(node1);
    freqList.addNode(node2);
    freqList.addNode(node3);

    // 验证首个节点始终为第一个添加的节点
    assert(freqList.getFirstNode() == node1);
    printf("TestMultipleNodes passed.\n");
}
// 测试节点移除后的场景
void TestAfterNodeRemoval() {
    FreqList<int, char> freqList(3);

    auto nodeA = std::make_shared<FreqList<int, char>::Node>(1, 'A');
    auto nodeB = std::make_shared<FreqList<int, char>::Node>(2, 'B');
    freqList.addNode(nodeA);
    freqList.addNode(nodeB);

    // 移除第一个节点
    freqList.removeNode(nodeA);

    // 验证新首个节点
    assert(freqList.getFirstNode() == nodeB);

    // 再移除所有节点
    freqList.removeNode(nodeB);
    assert(freqList.getFirstNode() == freqList.tail_);

    printf("TestAfterNodeRemoval passed.\n");
}
int main()
{
    //TestRemoveMiddleNode();
    //TestRemoveHeadSentinel();
    //TestRemoveTailSentinel();
    //TestGetFirstNode();
    //TestEmptyList();
    //TestSingleNode();
    //TestMultipleNodes();
    TestAfterNodeRemoval();
    return 0;
}