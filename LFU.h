#pragma once
#include "CachePolicy.h"

template<typename Key, typename Value> class LfuCache;

template<typename Key,typename Value>
class FreqList
{
private:
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

	friend class LfuCache<Key, Value>;
};

template <typename Key,typename Value>
class LfuCache : public FgCache::FgCachePolicy<Key,Value>
{
public:
	using Node =  typename FreqList<Key,Value>::Node;
	using NodePtr = std::shared_ptr<Node>;
	using NodeMap = std::unordered_map<Key, NodePtr>;
	LfuCache(int capacity,int maxAverageNum = 10)
		:capacity_(capacity),minFreq_(INT8_MAX),maxAverageNum_(maxAverageNum),
		curAverageNum_(0),curTotalNum_(0){}
	~LfuCache() override = default;

	void put(Key key, Value value) override
	{
		if (capacity_ == 0) return;
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = nodeMap_.find(key);
		if (it != nodeMap_.end())
		{
			// ������ value ֵ
			it->second->value = value;

			getInternal(it->second, value);
			return;
		}
		putInternal(key, value);
	}
	// value ֵΪ��������
	bool get(Key key, Value& value) override
	{
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = nodeMap_.find(key);
		if (it != nodeMap_.end())
		{
			getInternal(it->second, value);
			return true;
		}
		return false;
	}
	Value get(Key key) override
	{
		Value value;
		get(key, value);
		return value;
	}
	//��ջ��棬������Դ
	void purge()
	{
		nodeMap_.clear();
		freqToFreqList_.clear();
	}
private:
	void putInternal(Key key, Value value); // ��ӻ���
	void getInternal(NodePtr node, Value& value); // ��ȡ����
	void removeFromFreqList(NodePtr node); // ��Ƶ���б����Ƴ��ڵ�
	void addToFreqList(NodePtr node); // ��ӵ�Ƶ���б�
	void addFreqNum(); // ����ƽ�����ʵ�Ƶ��
	void handleOverMaxAverageNum(); // ����ǰ����Ƶ�ʳ������޵����
	void updateMinFreq();
	void kictOut(); // �Ƴ������еĹ�������
	void decreaseFreqNum(int num);
private:
	int capacity_;  //��������
	int minFreq_;   //��С����Ƶ�Σ������ҵ���С����Ƶ�ν�㣩
	int maxAverageNum_;  //���ƽ������Ƶ��
	int curAverageNum_;  //��ǰƽ������Ƶ��
	int curTotalNum_; // ��ǰ�������л����������
	std::mutex mutex_; // ������
	NodeMap nodeMap_; // key ���������ӳ��
	std::unordered_map<int, FreqList<Key, Value>*> freqToFreqList_;// ����Ƶ�ε���Ƶ�������ӳ��
};
template<typename Key,typename Value>
void LfuCache<Key, Value>::getInternal(NodePtr node, Value& value)
{
	//�ҵ�֮����Ҫ����ӵͷ���Ƶ�ε�������ɾ����������ӵ�+1�ķ���Ƶ��������
	//����Ƶ��+1��Ȼ���valueֵ����
	value = node->value;
	removeFromFreqList(node);
	node->freq++;
	addToFreqList(node);
	//�����ǰnode�ķ���Ƶ�ε���minFreq+1,������ǰ������Ϊ�գ���˵��
	//freqToFreqList_[node->freq-1]������node��Ǩ���Ѿ����ˣ���Ҫ������С����Ƶ��
	if (node->freq - 1 == minFreq_ && freqToFreqList_[node->freq - 1]->isEmpty())
	{
		minFreq_++;
	}
	//�ܷ���Ƶ�κ͵�ǰ����Ƶ�ζ���֮����
	addFreqNum();
}
template<typename Key,typename Value>
void LfuCache<Key, Value>::putInternal(Key key, Value value)
{
	// ������ڻ����У�����Ҫ�жϻ����Ƿ�����
	if (nodeMap_.size() == capacity_)
	{
		// ����������ɾ��������ʵĽ�㣬���µ�ǰƽ������Ƶ�κ��ܷ���Ƶ��
		kictOut();
	}
	// �����½ڵ㣬���½ڵ���Ӽ��룬������С����Ƶ��
	NodePtr node = std::make_shared<Node>(key, value);
	nodeMap_[key] = node;
	addToFreqList(node);
	addFreqNum();
	minFreq_ = std::min(minFreq_, 1);
}
template<typename Key, typename Value>
void LfuCache<Key, Value>::removeFromFreqList(NodePtr node)
{
	// ���ڵ��Ƿ�Ϊ��
	if (!node) return;
	auto freq = node->freq;
	freqToFreqList_[freq]->removeNode(node);
}


template<typename Key,typename Value>
void LfuCache<Key,Value>::addToFreqList(NodePtr node)
{
	// ���ڵ��Ƿ�Ϊ��
	if (!node) return;
	//��ӽ�����Ӧ��Ƶ������ǰ��Ҫ�жϸ�Ƶ�������Ƿ����
	auto freq = node->freq;
	if (freqToFreqList_.find(node->freq) == freqToFreqList_.end())
	{
		// �������򴴽�
		freqToFreqList_[node->freq] = new FreqList<Key, Value>(node->freq);
	}
	freqToFreqList_[freq]->addNode(node);
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::addFreqNum()
{
	curTotalNum_++;
	if (nodeMap_.empty())
	{
		curAverageNum_ = 0;
	}
	else
	{
		curAverageNum_ = curTotalNum_ / nodeMap_.size();
	}

	if (curAverageNum_ > maxAverageNum_)
	{
		handleOverMaxAverageNum();
	}
}

template<typename Key,typename Value>
void LfuCache<Key,Value>::handleOverMaxAverageNum()
{
	if(nodeMap_.empty()) return;
	//��ǰƽ������Ƶ���Ѿ����������ƽ������Ƶ�Σ����н��ķ���Ƶ��-(maxAverageNum_/2)
		for (auto it = nodeMap_.begin(); it != nodeMap_.end(); it++)
		{
			//������Ƿ�Ϊ��
			if (!it->second) continue;
			NodePtr node = it->second;
				// �ȴӵ�ǰƵ���б����Ƴ�
			removeFromFreqList(node);
				//����Ƶ��
			node->freq -= maxAverageNum_ / 2;
			if (node->freq < 1) node->freq = 1;
			//��ӵ��µ�Ƶ���б�
			addToFreqList(node);
		}
	// ������СƵ��
	updateMinFreq();
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::updateMinFreq()
{
	minFreq_ = INT8_MAX;
	for (const auto& pair : freqToFreqList_)
	{
		if (pair.second && !pair.second->isEmpty())
		{
			minFreq_ = std::min(minFreq_, pair.first);
		}
	}
	if (minFreq_ == INT8_MAX) minFreq_ = 1;
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::kictOut()
{
	NodePtr node = freqToFreqList_[minFreq_]->getFirstNode();
	removeFromFreqList(node);
	nodeMap_.erase(node->key);
	decreaseFreqNum(node->freq);
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::decreaseFreqNum(int num)
{
	// ����ƽ������Ƶ�κ��ܷ���Ƶ��
	curTotalNum_ -= num;
	if (nodeMap_.empty()) curAverageNum_ = 0;
	else curAverageNum_ = curTotalNum_ / nodeMap_.size();
}
