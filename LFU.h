#pragma once
#include "CachePolicy.h"

template<typename Key,typename Value>
class FreqList
{
public:
	struct Node
	{
		int freq; // ����Ƶ��
		Key key;
		Value value;
		std::shared_ptr<Node> pre; // ��һ���
		std::shared_ptr<Node> next;  

		Node()
			:freq(1),pre(nullptr),next(nullptr) {}
		Node(Key key, Value value)
			:freq(1),key(key),value(value),pre(nullptr),next(nullptr){}
	};
	using NodePtr = std::shared_ptr<Node>;
	int freq_; //����Ƶ��
	NodePtr head_; // ��ͷ���
	NodePtr tail_; //��β���
public:
	explicit FreqList(int n)
		: freq_(n)
	{
		head_ = std::make_shared<Node>();
		tail_ = std::make_shared<Node>();
		head_->next = tail_;
		tail_->pre = head_;
	}
	bool isEmpty() const
	{
		return head_->next == tail_;
	}
	void addNode(NodePtr node)
	{
		if (!node) return;
		node-> pre = tail_->pre;
		node->next = tail_;
		tail_->pre->next = node;
		tail_->pre = node;
	}
	void removeNode(NodePtr node)
	{
		if (!node) return;
		if (!node->pre || !node->next) return;
		node->pre->next = node->next;
		node->next->pre = node->pre;
		node->pre = nullptr;
		node->next = nullptr;
	}
	NodePtr getFirstNode() const
	{
		return head_->next;
	}
};