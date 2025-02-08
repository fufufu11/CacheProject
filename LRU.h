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
        size_t accessCount_; // 访问次数
        std::shared_ptr<LruNode<Key, Value>> prev_;
        std::shared_ptr<LruNode<Key, Value>> next_;

    public:
        LruNode(Key key, Value value)
            : key_(key), value_(value), accessCount_(1), prev_(nullptr), next_(nullptr)
        {
        }
        //提供必要的访问器
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
            // 创建收尾虚拟节点
            dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
            dummyHead_->next_ = dummyTail_;
            dummyTail_->prev_ = dummyHead_;
        }

        //添加缓存
        void put(Key key, Value value) override
        {
            if (capacity_ <= 0)
            {
                return;
            }
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = nodeMap_.find(key);
            if (it != nodeMap_.end()) //如果找到
            {
                // 如果在当前容器中，则更新 value
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
        //删除指定元素
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
        //将该节点移到最新位置
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
        //从尾部插入节点
        void insertNode(NodePtr node)
        {
            node->next_ = dummyTail_;
            node->prev_ = dummyTail_->prev_;
            dummyTail_->prev_->next_ = node;
            dummyTail_->prev_ = node;
        }
        //驱逐最近最少访问
        void evictLeastRecent()
        {
            NodePtr leastRecent = dummyHead_->next_;
            removeNode(leastRecent);
            nodeMap_.erase(leastRecent->getKey());
        }
    private:
        int capacity_; // 缓存容量
        NodeMap nodeMap_; // key -> Node
        std::mutex mutex_;
        NodePtr dummyHead_; // 虚拟头结点
        NodePtr dummyTail_;
    };


    //LRU 优化：LRU-K 版本。通过继承的方式优化
    template <typename Key, typename Value>
    class FLruCache_K : public FLruCache<Key, Value>
    {
    public:
        FLruCache_K(int capacity, int historyCapacity, int k)
            : FLruCache<Key, Value>(capacity)//调用基类构造
            , historyList_(std::make_unique<FLruCache<Key, size_t>>(historyCapacity))
            , k_(k)
        {}
        Value get(Key key)
        {
            // 获取该数据访问次数
            int historyCount = historyList_->get(key);
            // 如果访问到数据，则更新历史访问记录节点值 count++
            historyList_->put(key, ++historyCount);
            // 从缓存中获取数据，不一定能获取到，因为可能不在缓存中
            return FLruCache<Key, Value>::get(key);
        }
        void put(Key key, Value value)
        {
            //先判断是否存在于缓存中，如果存在于则直接覆盖，如果不存在则不直接添加到缓存
            if (FLruCache<Key, Value>::get(key) != "")
            {
                FLruCache<Key, Value>::put(key, value);
            }
            //如果数据历史访问次数达到上限，则添加入缓存
            int historyCount = historyList_->get(key);
            historyList_->put(key, ++historyCount);
            if (historyCount >= k_)
            {
                //移除历史访问记录
                historyList_->remove(key);
                //添加入缓存中
                FLruCache<Key, Value>::put(key, value);
            }
        }
    private:
        int k_; //进入缓存队列的评判标准
        std::unique_ptr<FLruCache<Key, size_t>> historyList_; // 访问数据历史记录 (value 为访问次数)
    };

    //lru 优化：对 lru 进行分片，提高高并发使用性能
    template<typename Key, typename Value>
    class HashLruCaches
    {
    public:
        HashLruCaches(size_t capacity, int sliceNum)
            : capacity_(capacity)
            , sliceNum_(sliceNum > 0 ? sliceNum : std::thread::hardware_concurrency()) //concurrency()获取系统支持的并发线程数
        {
            size_t sliceSize = std::ceil(capacity / static_cast<double>(sliceNum_));//获取每个分片的大小
            for (int i = 0; i < sliceNum_; i++)
            {
                lruSliceCaches_.emplace_back(new FLruCache<Key, Value>(sliceSize));
            }
        }
        void put(Key key, Value value)
        {
            // 获取 key 的 hash 值，并计算出对应的分片索引
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
        // 将 key 转换为对应的 hash 值
        size_t Hash(Key key)
        {
            std::hash<Key> hashFunc;
            return hashFunc(key);
        }
    private:
        size_t capacity_; //总容量
        int sliceNum_; //切片数量
        std::vector<std::unique_ptr<FLruCache<Key, Value>>> lruSliceCaches_; // 切片 LRU 缓存
    };
}