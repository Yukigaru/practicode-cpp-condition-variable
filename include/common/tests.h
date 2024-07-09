#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

using TestFunc = void (*)();

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
        throw std::runtime_error(msg); \
    }

#define EXPECT(expr) \
    if (!(expr)) { \
        std::string msg = #expr " at line " + std::to_string(__LINE__); \
        throw std::runtime_error(msg); \
    }

#define EXPECT_EQ(expr, expected) EXPECT_OP(expr, ==, expected)

#define EXPECT_GT(expr, expected) EXPECT_OP(expr, >, expected)

#define EXPECT_GE(expr, expected) EXPECT_OP(expr, >=, expected)

#define EXPECT_LT(expr, expected) EXPECT_OP(expr, <, expected)

#define EXPECT_LE(expr, expected) EXPECT_OP(expr, <=, expected)

#define EXPECT_TRUE(expr) EXPECT_OP(expr, ==, true)

#define EXPECT_FALSE(expr) EXPECT_OP(expr, ==, false)


#define REPEATED_TEST(testFunc, N) \
    void testFunc(); \
    void testFunc##_wrapper() { \
        std::atomic_bool finish{false}; \
        std::mutex m; \
        std::condition_variable cv; \
        auto start = std::chrono::steady_clock::now(); \
        std::thread watchdog([&]() { \
            std::unique_lock l{m}; \
            cv.wait(l, [&]() { \
                return finish.load() || \
                       std::chrono::steady_clock::now() - start > std::chrono::seconds(2); \
            }); \
            if (!finish.load()) { \
                std::cerr << "[FAIL] Test " #testFunc " can't proceed" << std::endl; \
                std::exit(1); \
            } \
        }); \
        bool pass{}; \
        try { \
            for (int i = 0; i < N; ++i) { \
                testFunc(); \
            } \
            pass = true; \
        } catch (const std::exception& e) { \
            std::cerr << "[FAIL] " #testFunc ": " << e.what() << std::endl; \
        } \
        { \
            std::unique_lock l{m}; \
            finish.store(true); \
            cv.notify_one(); \
        } \
        watchdog.join(); \
        if (pass) { \
            std::cout << "[PASS] " #testFunc << std::endl; \
        } \
    } \
    struct testFunc##_registrar { \
        testFunc##_registrar() { _all_tests.push_back(testFunc##_wrapper); } \
    } testFunc##_instance; \
    void testFunc()


#define TEST(testFunc) REPEATED_TEST(testFunc, 10)


#define RUN_TESTS() \
    for (TestFunc test : _all_tests) { \
        (*test)(); \
    }
