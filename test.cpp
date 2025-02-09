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


class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// ������������ӡ���
void printResults(const std::string& testName, int capacity,
    const std::vector<int>& get_operations,
    const std::vector<int>& hits)
{
    std::cout << "�����С: " << capacity << std::endl;
    std::cout << "LRU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[0] / get_operations[0]) << "%" << std::endl;
    std::cout << "LFU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[1] / get_operations[1]) << "%" << std::endl;
    std::cout << "ARC - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[2] / get_operations[2]) << "%" << std::endl;
}
void testHotDataAccess() {
    std::cout << "\n=== ���Գ���1���ȵ����ݷ��ʲ��� ===" << std::endl;

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

    // �Ƚ���һϵ��put����
    for (int i = 0; i < caches.size(); ++i) 
    {
        for (int op = 0; op < OPERATIONS; ++op)
        {
            int key;
            if (op % 100 < 80) 
            {  // 80%�ȵ�����
                key = gen() % HOT_KEYS;
            }
            else 
            {  // 20%������
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ��������get����
        for (int get_op = 0; get_op < OPERATIONS / 2; ++get_op) 
        {
            int key;
            if (get_op % 100 < 80) 
            {  // 80%���ʷ����ȵ�
                key = gen() % HOT_KEYS;
            }
            else
            {  // 20%���ʷ���������
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
    printResults("�ȵ����ݷ��ʲ���", CAPACITY, get_operations, hits);
}

void testLoopPattern() {
    std::cout << "\n=== ���Գ���2��ѭ��ɨ����� ===" << std::endl;

    const int CAPACITY = 3;
    const int LOOP_SIZE = 200;
    const int OPERATIONS = 50000;

    FgCache::FLruCache<int, std::string> lru(CAPACITY);
    FgCache::LfuCache<int, std::string> lfu(CAPACITY);
    FgCache::ArcCache<int, std::string> arc(CAPACITY);

    std::array<FgCache::FgCachePolicy<int, std::string>*, 3> caches = { &lru, &lfu, &arc };
    std::vector<int> hits(3, 0);
    std::vector<int> get_operations(3, 0);

    std::random_device rd;
    std::mt19937 gen(rd());

    // ���������
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < LOOP_SIZE * 2; ++key) {
            std::string value = "loop" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ����з��ʲ���
        int current_pos = 0;
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 70) {  // 70%˳��ɨ��
                key = current_pos;
                current_pos = (current_pos + 1) % LOOP_SIZE;
            }
            else if (op % 100 < 85) {  // 15%�����Ծ
                key = gen() % LOOP_SIZE;
            }
            else {  // 15%���ʷ�Χ������
                key = LOOP_SIZE + (gen() % LOOP_SIZE);
            }

            std::string result;
            get_operations[i]++;
            if (caches[i]->get(key, result)) {
                hits[i]++;
            }
        }
    }

    printResults("ѭ��ɨ�����", CAPACITY, get_operations, hits);
}

void testWorkloadShift() {
    std::cout << "\n=== ���Գ���3���������ؾ��ұ仯���� ===" << std::endl;

    const int CAPACITY = 4;
    const int OPERATIONS = 80000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    FgCache::FLruCache<int, std::string> lru(CAPACITY);
    FgCache::LfuCache<int, std::string> lfu(CAPACITY);
    FgCache::ArcCache<int, std::string> arc(CAPACITY);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<FgCache::FgCachePolicy<int, std::string>*, 3> caches = { &lru, &lfu, &arc };
    std::vector<int> hits(3, 0);
    std::vector<int> get_operations(3, 0);

    // �����һЩ��ʼ����
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < 1000; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ����ж�׶β���
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            // ���ݲ�ͬ�׶�ѡ��ͬ�ķ���ģʽ
            if (op < PHASE_LENGTH) {  // �ȵ����
                key = gen() % 5;
            }
            else if (op < PHASE_LENGTH * 2) {  // ��Χ���
                key = gen() % 1000;
            }
            else if (op < PHASE_LENGTH * 3) {  // ˳��ɨ��
                key = (op - PHASE_LENGTH * 2) % 100;
            }
            else if (op < PHASE_LENGTH * 4) {  // �ֲ������
                int locality = (op / 1000) % 10;
                key = locality * 20 + (gen() % 20);
            }
            else {  // ��Ϸ���
                int r = gen() % 100;
                if (r < 30) {
                    key = gen() % 5;
                }
                else if (r < 60) {
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

            // �������put���������»�������
            if (gen() % 100 < 30) {  // 30%���ʽ���put
                std::string value = "new" + std::to_string(key);
                caches[i]->put(key, value);
            }
        }
    }

    printResults("�������ؾ��ұ仯����", CAPACITY, get_operations, hits);
}
int main() 
{
    testHotDataAccess();
    //testLoopPattern();
   // testWorkloadShift();
    return 0;
}