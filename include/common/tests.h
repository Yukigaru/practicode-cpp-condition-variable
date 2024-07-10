#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using TestFunc = bool (*)();

std::vector<TestFunc> _all_tests;

namespace std {
template <typename Rep, typename Period>
std::string to_string(const std::chrono::duration<Rep, Period>& duration) {
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) + "ms";
}
}

#define EXPECT_OP(expr, op, expected) \
    if (!((expr) op (expected))) { \
        std::string msg = "`" #expr "` got: " + std::to_string(expr) + ", expected: " + std::to_string(expected) + " at line " + std::to_string(__LINE__); \
        std::string out_msg = "[FAIL] " + ctx.func_name + ": " + msg + "\n"; \
        std::cerr << out_msg; \
        throw std::runtime_error(msg); \
    }

#define EXPECT(expr) \
    if (!(expr)) { \
        std::string msg = #expr " at line " + std::to_string(__LINE__); \
        std::string out_msg = "[FAIL] " + ctx.func_name + ": " + msg + "\n"; \
        std::cerr << out_msg; \
        throw std::runtime_error(msg); \
    }

#define EXPECT_EQ(expr, expected) EXPECT_OP(expr, ==, expected)

#define EXPECT_GT(expr, expected) EXPECT_OP(expr, >, expected)

#define EXPECT_GE(expr, expected) EXPECT_OP(expr, >=, expected)

#define EXPECT_LT(expr, expected) EXPECT_OP(expr, <, expected)

#define EXPECT_LE(expr, expected) EXPECT_OP(expr, <=, expected)

#define EXPECT_TRUE(expr) EXPECT_OP(expr, ==, true)

#define EXPECT_FALSE(expr) EXPECT_OP(expr, ==, false)


struct TestContext {
    std::string func_name;
};

// Макрос запускает тест N раз, а так же вызывает exit, если тест не завершился за 2 секунды
#define REPEATED_TEST(testFunc, N) \
    void testFunc(const TestContext &); \
    bool testFunc##_wrapper() { \
        bool finish{false}; \
        std::mutex m; \
        std::condition_variable cv; \
        auto start = std::chrono::steady_clock::now(); \
        std::thread watchdog([&]() { \
            std::unique_lock l{m}; \
            cv.wait(l, [&]() { \
                return finish || \
                       std::chrono::steady_clock::now() - start > std::chrono::seconds(2); \
            }); \
            if (!finish) { \
                std::cerr << "[FAIL] Test " #testFunc " can't proceed" << std::endl; \
                std::exit(1); \
            } \
        }); \
        bool pass{}; \
        try { \
            TestContext ctx{ std::string{ #testFunc } }; \
            for (int i = 0; i < N; ++i) { \
                testFunc(ctx); \
            } \
            pass = true; \
        } catch (const std::exception& e) { \
            \
        } \
        { \
            std::unique_lock l{m}; \
            finish = true; \
            cv.notify_one(); \
        } \
        watchdog.join(); \
        if (pass) { \
            std::cout << "[PASS] " #testFunc << std::endl; \
        } \
        return pass; \
    } \
    struct testFunc##_registrar { \
        testFunc##_registrar() { _all_tests.push_back(testFunc##_wrapper); } \
    } testFunc##_instance; \
    void testFunc(const TestContext &ctx)


#define TEST(testFunc) REPEATED_TEST(testFunc, 10)


#define RUN_TESTS() \
    try { \
        for (TestFunc test : _all_tests) { \
            auto pass = (*test)(); \
            if (!pass) { \
                return 1; \
            } \
        } \
    \
    } catch (const std::exception& e) { \
        std::cerr << e.what() << std::endl; \
        return 1; \
    }
