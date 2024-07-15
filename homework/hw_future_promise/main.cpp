#include <mutex>
#include <condition_variable>
#include <exception>
#include <chrono>
#include <memory>
#include <iostream>
#include "tests.h"

using namespace std::chrono_literals;


class FutureError {
    // Extend the class if needed
};


template <typename T>
class Future {
public:
    // Waits until the shared state is ready, then retrieves the value stored in the shared state (if any). Right after calling this function, valid() is false.
    // If an exception was stored in the shared state referenced by the future (e.g. via a call to std::promise::set_exception()) then that exception will be thrown.
    T get() {
        // TODO: ...
    }

    // Blocks until the result becomes available.
    // valid() == true after the call.
    // if valid() == false before the call to this function, FutureError is thrown.
    void wait() const {
    }

    // returns true, if the result is ready, false otherwise (when timed out)
    template<class Rep, class Period>
    bool wait_for( const std::chrono::duration<Rep,Period>& timeout_duration) const {
    }


    // A Future object is considered "valid" if it is associated with a shared state. This happens when:
    // - The std::future is created by a std::promise (using std::promise::get_future()).
    // - The std::future has not yet been used to retrieve the value or shared state (using get() or share()).
    bool valid() const noexcept {
        return false;
    }

private:

};


template <typename T>
class Promise {
public:
    // Calls to this function do not introduce data races with calls to get_future (therefore they need not synchronize with each other).
    Future<T> get_future() {
        // TODO: ...
        return Future<T>{};
    }

    // Throws std::future_error on the following conditions:
    // - *this has no shared state
    // - The shared state already stores a value or exception. The error code is set to promise_already_satisfied.
    // Additionally:
    // 1) Any exception thrown by the constructor selected to copy an object of type R.
    // 2) Any exception thrown by the constructor selected to move an object of type R.
    void set_value(const T& value) {
        // TODO: ...
    }

    void set_value(T&& value) {
        // TODO: ...
    }

    // Atomically stores the exception pointer p into the shared state and makes the state ready.
    // The operation behaves as though set_value, set_exception, set_value_at_thread_exit, and set_exception_at_thread_exit acquire a single mutex associated with the promise object while updating the promise object.
    // An exception is thrown if there is no shared state or the shared state already stores a value or exception.
    // Calls to this function do not introduce data races with calls to get_future (therefore they need not synchronize with each other).
    void set_exception(std::exception_ptr e) {
        // TODO: ...
    }

private:

};


/*
 * Тесты
 */
TEST(test_promise_set_value) {
    Promise<int> promise;
    auto future = promise.get_future();

    promise.set_value(2);
    auto result = future.get();

    EXPECT_EQ(result, 2);
}

TEST(test_promise_wrong_set_value) {
    Promise<int> promise;

    EXPECT_THROW(promise.set_value(0), FutureError);

    auto future = promise.get_future();

    EXPECT_EQ(future.get(), 0);

    EXPECT_THROW(future.wait(), FutureError);
    EXPECT_THROW(future.get(), FutureError);
}

TEST(test_promise_wrong_set_value_2) {
    Promise<int> promise;
    Future<int> future = promise.get_future();

    promise.set_value(62);

    EXPECT_THROW(promise.set_value(73), FutureError);
}

TEST(test_valid) {
    // 1
    Future<int> lost_future;
    EXPECT_FALSE(lost_future.valid());

    // 2
    Promise<int> promise;
    auto future = promise.get_future();

    EXPECT_TRUE(future.valid());

    promise.set_value(4);

    EXPECT_TRUE(future.valid());
    auto result = future.get();

    EXPECT_FALSE(future.valid());
}

TEST(test_future_wait) {
    Promise<int> promise;
    auto future = promise.get_future();

    std::thread t([&promise]() {
        std::this_thread::sleep_for(50s);
        promise.set_value(1);
    });

    future.wait();
    auto result = future.get();
    EXPECT_EQ(result, 1);

    t.join();
}

TEST(test_future_wait_for) {
    Promise<int> promise;
    auto future = promise.get_future();

    std::thread t([&promise]() {
        std::this_thread::sleep_for(100ms);
        promise.set_value(11);
    });

    auto status = future.wait_for(60ms);
    EXPECT_FALSE(status);

    status = future.wait_for(60ms);
    EXPECT_TRUE(status);
    auto result = future.get();
    EXPECT_EQ(result, 11);

    t.join();
}

TEST(test_future_exception) {
    Promise<int> promise;
    Future<int> future = promise.get_future();

    promise.set_exception(std::make_exception_ptr(std::runtime_error("test error")));

    try {
        int result = future.get();
        EXPECT(false); // не должны сюда попасть

    } catch (const std::runtime_error& e) {
        EXPECT_EQ(e.what(), "test error");
    }

    EXPECT_FALSE(future.valid());
}

int main() {
    RUN_TESTS();
    return 0;
}
/* Усложнения:
 * - поддержи Future<void> вариант, аналогично тому как это сделано в std::future
 *
 * */
