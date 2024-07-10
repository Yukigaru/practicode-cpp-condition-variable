#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include "tests.h"

// call_once примитив позволяет вызвать callback функцию гарантированно только один раз, даже, если вывов осуществляется параллельно из нескольких потоков.
// Параллельные вызовы call_once должны подождать, пока первый вызов завершится, гарантирую тем самым, что все side-effect'ы сделанные callback'ом будут увидены.
// Аналогичен тому, как работает std::call_once.
class OnceFlag {
public:
};

template<typename Callable>
void call_once(OnceFlag &flag, Callable&& func) {
    // ...
}


/*
 * Тесты
 * */
TEST(test_single_call) {
    OnceFlag flag;
    int counter = 0;

    call_once(flag, [&counter]() {
        ++counter;
    });

    EXPECT_EQ(counter, 1);
}

TEST(test_multiple_threads) {
    OnceFlag flag;
    std::atomic_int counter = 0;
    static constexpr int num_threads = 16;

    auto increment = [&flag, &counter]() {
        for (int i = 0; i < 10; i++) {
            call_once(flag, [&counter]() {
                counter++;
            });
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(increment);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(counter, 1);
}

int main() {
    RUN_TESTS();
    return 0;
}

