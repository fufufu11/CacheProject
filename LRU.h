#pragma once
#pragma once
#include <iostream>
#include <memory>
#include <unordered_map>
#include <mutex>
#include "CachePolicy.h"
namespace FgCache
{
    template<typename Key, typename Value> class FLruCache;
    template <typename Key, typename Value>
    class LruNode
    {
    private:
        Key key_;
        Value value_;
        size_t accessCount_; // ���ʴ���
        std::shared_ptr<LruNode<Key, Value>> prev_;
        std::shared_ptr<LruNode<Key, Value>> next_;

    public:
        LruNode(Key key, Value value)
            : key_(key), value_(value), accessCount_(1), prev_(nullptr), next_(nullptr)
        {
        }
        //�ṩ��Ҫ�ķ�����
        Key getKey() const { return key_; }
        Value getValue() const { return value_; }
        void setValue(const Value& value) { value_ = value; }
        size_t getAccessCount() const { return accessCount_; }
        void incrementAccessCount() { ++accessCount_; }

        friend class FLruCache<Key, Value>;
    };

    template<typename Key, typename Value>
    class FLruCache : public FgCache::FgCachePolicy<Key, Value>
    {
    public:
        using LruNodeType = LruNode<Key, Value>;
        using NodePtr = std::shared_ptr<LruNodeType>;
        using NodeMap = std::unordered_map<Key, NodePtr>;

        FLruCache(int capacity) : capacity_(capacity)
        {
            initializeList();
        }
        ~FLruCache() override = default;
        void initializeList()
        {
            // ������β����ڵ�
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }

        //��ӻ���
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0)
            {
                return;
            }
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) //����ҵ�
            {
                // ����ڵ�ǰ�����У������ value
                updateExistingNode(it->second, value);
                return;
            }
            addNewNode(key, value);
        }
        bool get(Key key, Value& value) override
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                moveToMostRecent(it->second);
                value = it->second->getValue();
                return true;
            }
            return false;
        }
        Value get(Key key) override
        {
            Value value{};
            get(key, value);
            return value;
        }
        //ɾ��ָ��Ԫ��
        void remove(Key key)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end())
            {
                removeNode(it->second);
                nodeMap_.erase(it);
            }
        }
    private:
        void addNewNode(const Key& key, const Value& value)
        {
            if (nodeMap_.size() >= capacity_)
            {
                evictLeastRecent();
            }
            NodePtr newNode = std::make_shared<LruNodeType>(key, value);
            insertNode(newNode);
            nodeMap_[key] = newNode;
        }
        void updateExistingNode(NodePtr node, const Value& value)
        {
            node->setValue(value);
            moveToMostRecent(node);
        }
        //���ýڵ��Ƶ�����λ��
        void moveToMostRecent(NodePtr node)
        {
            removeNode(node);
            insertNode(node);
        }
        void removeNode(NodePtr node)
        {
            node->prev_->next_ = node->next_;
            node->next_->prev_ = node->prev_;
        }
        //��β������ڵ�
        void insertNode(NodePtr node)
        {
            node->next_ = dummyTail_;
            node->prev_ = dummyTail_->prev_;
            dummyTail_->prev_->next_ = node;
            dummyTail_->prev_ = node;
        }
        //����������ٷ���
        void evictLeastRecent()
        {
            NodePtr leastRecent = dummyHead_->next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent->getKey());
        }
    private:
        int capacity_; // ��������
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_; // ����ͷ���
        NodePtr dummyTail_;
    };


    //LRU �Ż���LRU-K �汾��ͨ���̳еķ�ʽ�Ż�
    template <typename Key, typename Value>
    class FLruCache_K : public FLruCache<Key, Value>
    {
    public:
        FLruCache_K(int capacity, int historyCapacity, int k)
            : FLruCache<Key, Value>(capacity)//���û��๹��
            , historyList_(std::make_unique<FLruCache<Key, size_t>>(historyCapacity))
            , k_(k)
        {}
        Value get(Key key)
        {
            // ��ȡ�����ݷ��ʴ���
            int historyCount = historyList_->get(key);
            // ������ʵ����ݣ��������ʷ���ʼ�¼�ڵ�ֵ count++
            historyList_->put(key, ++historyCount);
            // �ӻ����л�ȡ���ݣ���һ���ܻ�ȡ������Ϊ���ܲ��ڻ�����
            return FLruCache<Key, Value>::get(key);
        }
        void put(Key key, Value value)
        {
            //���ж��Ƿ�����ڻ����У������������ֱ�Ӹ��ǣ������������ֱ����ӵ�����
            if (FLruCache<Key, Value>::get(key) != "")
            {
                FLruCache<Key, Value>::put(key, value);
            }
            //���������ʷ���ʴ����ﵽ���ޣ�������뻺��
            int historyCount = historyList_->get(key);
            historyList_->put(key, ++historyCount);
            if (historyCount >= k_)
            {
                //�Ƴ���ʷ���ʼ�¼
                historyList_->remove(key);
                //����뻺����
                FLruCache<Key, Value>::put(key, value);
            }
        }
    private:
        int k_; //���뻺����е����б�׼
        std::unique_ptr<FLruCache<Key, size_t>> historyList_; // ����������ʷ��¼ (value Ϊ���ʴ���)
    };

    //lru �Ż����� lru ���з�Ƭ����߸߲���ʹ������
    template<typename Key, typename Value>
    class HashLruCaches
    {
    public:
        HashLruCaches(size_t capacity, int sliceNum)
            : capacity_(capacity)
            , sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency()) //concurrency()��ȡϵͳ֧�ֵĲ����߳���
        {
            size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum_));//��ȡÿ����Ƭ�Ĵ�С
            for (int i = 0; i < sliceNum_; i++)
            {
                lruSliceCaches_.emplace_back(new FLruCache<Key, Value>(sliceSize));
            }
        }
        void put(Key key, Value value)
        {
            // ��ȡ key �� hash ֵ�����������Ӧ�ķ�Ƭ����
            size_t sliceIndex = Hash(key) % sliceNum_;
            lruSliceCaches_[sliceIndex]->put(key, value);
        }
        bool get(Key key, Value& value)
        {
            size_t sliceIndex = Hash(key) % sliceNum_;
            return lruSliceCaches_[sliceIndex]->get(key, value);
        }
        Value get(Key key)
        {
            Value value;
            memset(&value, 0, sizeof(value));
            get(key, value);
            return value;
        }
    private:
        // �� key ת��Ϊ��Ӧ�� hash ֵ
        size_t Hash(Key key)
        {
            std::hash<Key> hashFunc;
            return hashFunc(key);
        }
    private:
        size_t capacity_; //������
        int sliceNum_; //��Ƭ����
        std::vector<std::unique_ptr<FLruCache<Key, Value>>> lruSliceCaches_; // ��Ƭ LRU ����
    };
}