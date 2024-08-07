#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include "tests.h"

template <typename T>
class ConcurrentFIFOQueue {
public:
    // добавлен лимит на размер очереди
    explicit ConcurrentFIFOQueue(size_t limit = 0) : _limit(limit) {}

    void push(const T& val) {
        std::unique_lock l{_m};
        // ...
    }

    T pop() {
        std::unique_lock l{_m};
        // ...
        return T{};
    }

private:
    std::mutex _m;

    std::queue<T> _queue;
    size_t _limit;
};

/*
 * Тесты
 */
TEST(test_multiple_push_pop) {
    ConcurrentFIFOQueue<int> queue;

    queue.push(1);
    queue.push(2);
    queue.push(3);
    EXPECT_EQ(queue.pop(), 1);
    EXPECT_EQ(queue.pop(), 2);
    EXPECT_EQ(queue.pop(), 3);
}

TEST(test_pop_wait) {
    ConcurrentFIFOQueue<int> queue;
    std::atomic<bool> item_popped{false};

    std::thread consumer{[&]() {
        queue.pop();
        item_popped.store(true);
    }};

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(item_popped.load());

    queue.push(1);

    consumer.join();

    EXPECT_TRUE(item_popped.load());
}

TEST(test_push_wait) {
    // Проверяем, что push блокируется, если очередь переполнена
    constexpr auto Limit = 2u;
    ConcurrentFIFOQueue<int> queue{Limit};

    std::atomic_int values_pushed{0};

    std::thread producer([&]() {
        for (int i = 0; i < Limit + 1; ++i) {
            queue.push(i);
            values_pushed++;
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Добавлено в очередь `Limit` элементов, и один `push` в ожидании
    EXPECT_EQ(values_pushed.load(), Limit);

    // Разблокируем оставшийся поток
    queue.pop();
    producer.join();

    EXPECT_EQ(values_pushed.load(), Limit + 1);
}

TEST(test_multiple_threads) {
    constexpr auto NumThreads = 4;
    constexpr auto N = 100;  // каждый producer поток производит N чисел

    ConcurrentFIFOQueue<int> queue{2};  // лимит в 2 элемента

    std::vector<int> consumed;
    std::mutex consumed_mutex;

    auto producer_func = [&](int thread_num) {
        for (int i = 0; i < N; ++i) {
            int num = thread_num * N + i;
            queue.push(num);
        }
    };

    auto consumer_func = [&]() {
        for (int i = 0; i < N; ++i) {
            int num = queue.pop();

            std::lock_guard<std::mutex> lock(consumed_mutex);
            consumed.push_back(num);
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < NumThreads; ++i) {
        threads.emplace_back(producer_func, i);
        threads.emplace_back(consumer_func);
    }

    for (auto& t : threads) {
        t.join();
    }

    EXPECT_EQ(consumed.size(), N * NumThreads);

    std::sort(std::begin(consumed), std::end(consumed));

    for (int i = 1; i < N; ++i) {
        EXPECT_EQ(consumed[i], consumed[i - 1] + 1);
    }
}

int main() {
    RUN_TESTS();
    return 0;
}
/* Усложнения:
 * - добавь метод `size_t push_bulk(in_iter, max_count)`, позволяющий добавить несколько элементов за один вызов,
 *   и возвращающий количество успешно добавленных элементов. Метод так же ждёт в случае переполненной очереди,
 *   после чего пытается вставить максимально возможное в данный момент времени количество элементов.
 *
 * - добавь аналогичный метод `size_t pop_bulk(out_iter, max_count)`.
 *
 * - добавь методы
 *     bool push(const T& val, std::chrono::steady_clock::duration timeout) и
 *     bool pop(T& out, std::chrono::steady_clock::duration timeout),
 *   работающие не дольше заданного времени, и возвращающие false в случае таймаута.
 *
 */
