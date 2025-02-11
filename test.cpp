#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <array>
#include <string>
#include "LRU.h"
#include "LFU.h"
#include "ArcCacheNode.h"
#include "ArcLruPart.h"
#include "ArcLfuPart.h"
#include "ArcCache.h"
#include "CachePolicy.h"
using namespace FgCache;



// 辅助函数：打印结果
void printResults(const std::string& testName, const int capacity,
    const std::vector<int>& get_operations,
    const std::vector<int>& hits)
{
    std::cout << "缓存大小: " << capacity << std::endl;
    std::cout << "LRU - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[0] / get_operations[0]) << "%" << std::endl;
    std::cout << "LFU - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[1] / get_operations[1]) << "%" << std::endl;
    std::cout << "ARC - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[2] / get_operations[2]) << "%" << std::endl;
}
void testHotDataAccess() {
    std::cout << "\n=== 测试场景1：热点数据访问测试 ===" << std::endl;

    const int CAPACITY = 20;      
    const int OPERATIONS = 100000;
    const int HOT_KEYS = 9;       
    const int COLD_KEYS = 5000;     

    FgCache::FLruCache<int, std::string> lru(CAPACITY);
    FgCache::LfuCache<int, std::string> lfu(CAPACITY);
    //KamaCache::KLfuCache<int, std::string> lfu(CAPACITY);
    FgCache::ArcCache<int, std::string> arc(CAPACITY);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<FgCache::FgCachePolicy<int, std::string>*, 3> caches = { &lru, &lfu, &arc };
    std::vector<int> hits(3, 0);
    std::vector<int> get_operations(3, 0);

    // 先进行一系列put操作
    for (int i = 0; i < caches.size(); ++i) 
    {
        for (int op = 0; op < OPERATIONS; ++op)
        {
            int key;
            if (op % 100 < 80) 
            {  // 80%热点数据
                key = gen() % HOT_KEYS;
            }
            else 
            {  // 20%冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行随机get操作
        for (int get_op = 0; get_op < OPERATIONS / 2; ++get_op) 
        {
            int key;
            if (get_op % 100 < 80) 
            {  // 80%概率访问热点
                key = gen() % HOT_KEYS;
            }
            else
            {  // 20%概率访问冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string result;
            get_operations[i]++;
            if (caches[i]->get(key, result))
            {
                hits[i]++;
            }
        }
    }
    printResults("热点数据访问测试", CAPACITY, get_operations, hits);
}

void testLoopPattern() {
    std::cout << "\n=== 测试场景2：循环扫描测试 ===" << std::endl;

    const int CAPACITY = 30;        // 扩大缓存容量
    const int LOOP_SIZE = 100;      // 缩小循环规模
    const int OPERATIONS = 50000;  // 增加操作次数
    const int WARMUP_ROUNDS = 2;    // 预热轮数

    FgCache::FLruCache<int, std::string> lru(CAPACITY);
    FgCache::LfuCache<int, std::string> lfu(CAPACITY);
    FgCache::ArcCache<int, std::string> arc(CAPACITY);

    std::array<FgCache::FgCachePolicy<int, std::string>*, 3> caches = { &lru, &lfu, &arc };
    std::vector<int> hits(3, 0);
    std::vector<int> get_operations(3, 0);
    // 统一预热过程
    auto warmup = [&](auto& cache) {
        for (int r = 0; r < WARMUP_ROUNDS; ++r) {
            for (int i = 0; i < LOOP_SIZE; ++i) {
                cache.put(i, "warmup");
                if (i % 10 == 0) { // 创建热点
                    std::string tmp;
                    cache.get(i, tmp);
                }
            }
        }
    };

    // 测试逻辑
    auto testLogic = [&](auto& cache, int idx) {
        std::random_device rd;
        std::mt19937 gen(rd());
        int current_pos = 0;

        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 60) {       // 60%顺序扫描
                key = current_pos;
                current_pos = (current_pos + 1) % LOOP_SIZE;
            }
            else if (op % 100 < 80) { // 20%热点访问
                key = gen() % 10;      // 前10个为热点
            }
            else if (op % 100 < 95) { // 15%随机访问
                key = gen() % LOOP_SIZE;
            }
            else {                   // 5%外部数据
                key = LOOP_SIZE + (gen() % LOOP_SIZE);
            }

            std::string result;
            if (cache.get(key, result)) {
                hits[idx]++;
            }
            else {
                cache.put(key, "value");
            }
            get_operations[idx]++;
        }
    };

    // 执行测试
    for (int i = 0; i < caches.size(); ++i) {
        warmup(*caches[i]);
        testLogic(*caches[i], i);
    }

    printResults("优化循环测试", CAPACITY, get_operations, hits);
}

void testWorkloadShift() {
    
    std::cout << "\n=== 测试场景3：工作负载剧烈变化测试 ===" << std::endl;

    const int CAPACITY = 40;  // 增加缓存容量
    const int OPERATIONS = 80000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    FgCache::FLruCache<int, std::string> lru(CAPACITY);
    FgCache::LfuCache<int, std::string> lfu(CAPACITY);
    FgCache::ArcCache<int, std::string> arc(CAPACITY, 5);  // 增大transformThreshold，优化ARC

    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<FgCache::FgCachePolicy<int, std::string>*, 3> caches = { &lru, &lfu, &arc };
    std::vector<int> hits(3, 0);
    std::vector<int> get_operations(3, 0);

    // 先填充一些初始数据
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < 1000; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行多阶段测试
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            // 根据不同阶段选择不同的访问模式
            if (op < PHASE_LENGTH) {  // 热点访问
                key = gen() % 5;
            }
            else if (op < PHASE_LENGTH * 2) {  // 大范围随机
                key = gen() % 1000;
            }
            else if (op < PHASE_LENGTH * 3) {  // 顺序扫描
                key = (op - PHASE_LENGTH * 2) % 100;
            }
            else if (op < PHASE_LENGTH * 4) {  // 局部性随机
                int locality = (op / 1000) % 10;
                key = locality * 20 + (gen() % 20);
            }
            else {  // 混合访问
                int r = gen() % 100;
                if (r < 40) {  // 调整热点访问的比例
                    key = gen() % 5;
                }
                else if (r < 70) {
                    key = 5 + (gen() % 95);
                }
                else {
                    key = 100 + (gen() % 900);
                }
            }

            std::string result;
            get_operations[i]++;
            if (caches[i]->get(key, result)) {
                hits[i]++;
            }

            // 随机进行put操作，更新缓存内容
            if (gen() % 100 < 40) {  //40%概率进行put
                std::string value = "new" + std::to_string(key);
                caches[i]->put(key, value);
            }
        }
    }

    printResults("工作负载剧烈变化测试", CAPACITY, get_operations, hits);
}
int main() 
{
    testHotDataAccess();
    testLoopPattern();
    testWorkloadShift();
    return 0;
}