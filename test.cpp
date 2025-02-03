#include <iostream>
#include <cassert>
#include "LRU.h"
#include "LFU.h"

// ���������Ƴ��м�ڵ�
void TestRemoveMiddleNode() {
    FreqList<int, std::string> freqList(1);

    // �����������Խڵ㲢��ӵ�����
    auto node1 = std::make_shared<FreqList<int, std::string>::Node>(1, "A");
    auto node2 = std::make_shared<FreqList<int, std::string>::Node>(2, "B");
    auto node3 = std::make_shared<FreqList<int, std::string>::Node>(3, "C");
    freqList.addNode(node1);
    freqList.addNode(node2);
    freqList.addNode(node3);

    // ��֤��ʼ���ӹ�ϵ
    assert(node1->next == node2);
    assert(node2->pre == node1);
    assert(node2->next == node3);
    assert(node3->pre == node2);

    // �Ƴ��м�ڵ�
    freqList.removeNode(node2);

    // ��֤�����ӹ�ϵ
    assert(node1->next == node3);
    assert(node3->pre == node1);

    // ��֤���Ƴ��ڵ��ָ���ÿ�
    assert(node2->pre == nullptr);
    assert(node2->next == nullptr);

    printf("TestRemoveMiddleNode passed.\n");
}
// �����Ƴ�ͷ�ڵ㣨�Ƿ�������
void TestRemoveHeadSentinel() {
    FreqList<int, int> freqList(1);

    // �����Ƴ�ͷ�ڱ��ڵ�
    freqList.removeNode(freqList.head_);

    // ��֤�ڱ��ڵ���Ȼ����
    assert(freqList.head_->next == freqList.tail_);
    assert(freqList.tail_->pre == freqList.head_);

    printf("TestRemoveHeadSentinel passed.\n");
}
// �����Ƴ�β�ڵ㣨�Ƿ�������
void TestRemoveTailSentinel() {
    FreqList<int, double> freqList(2);

    // �����Ƴ�β�ڱ��ڵ�
    freqList.removeNode(freqList.tail_);

    // ��֤����ṹ����
    assert(freqList.head_->next == freqList.tail_);
    assert(freqList.tail_->pre == freqList.head_);

    printf("TestRemoveTailSentinel passed.\n");
}
// �����Ƴ�������������Ľڵ�
void TestRemoveOrphanNode() {
    FreqList<int, char> freqList(3);

    // �������������ڵ�
    auto nodeA = std::make_shared<FreqList<int, char>::Node>(1, 'X');
    auto nodeB = std::make_shared<FreqList<int, char>::Node>(2, 'Y');

    // ��nodeA��������
    freqList.addNode(nodeA);

    // �����Ƴ�δ���ӵ�nodeB
    freqList.removeNode(nodeB);  // Ӧ�����Ƴ�

    // ��֤����ṹ����
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
// ���Կ�����״̬
void TestEmptyList() {
    FreqList<int, std::string> freqList(3);

    // ��֤��������β�ڱ�
    assert(freqList.getFirstNode() == freqList.tail_);
    printf("TestEmptyList passed.\n");
}
// ���Ե��ڵ㳡��
void TestSingleNode() {
    FreqList<int, double> freqList(2);

    auto node = std::make_shared<FreqList<int, double>::Node>(1, 3.14);
    freqList.addNode(node);

    // ��֤��ȡ�׸��ڵ�
    assert(freqList.getFirstNode() == node);
    printf("TestSingleNode passed.\n");
}
// ���Զ�ڵ㳡��
void TestMultipleNodes() {
    FreqList<std::string, int> freqList(5);

    auto node1 = std::make_shared<FreqList<std::string, int>::Node>("first", 100);
    auto node2 = std::make_shared<FreqList<std::string, int>::Node>("second", 200);
    auto node3 = std::make_shared<FreqList<std::string, int>::Node>("third", 300);

    freqList.addNode(node1);
    freqList.addNode(node2);
    freqList.addNode(node3);

    // ��֤�׸��ڵ�ʼ��Ϊ��һ����ӵĽڵ�
    assert(freqList.getFirstNode() == node1);
    printf("TestMultipleNodes passed.\n");
}
// ���Խڵ��Ƴ���ĳ���
void TestAfterNodeRemoval() {
    FreqList<int, char> freqList(3);

    auto nodeA = std::make_shared<FreqList<int, char>::Node>(1, 'A');
    auto nodeB = std::make_shared<FreqList<int, char>::Node>(2, 'B');
    freqList.addNode(nodeA);
    freqList.addNode(nodeB);

    // �Ƴ���һ���ڵ�
    freqList.removeNode(nodeA);

    // ��֤���׸��ڵ�
    assert(freqList.getFirstNode() == nodeB);

    // ���Ƴ����нڵ�
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