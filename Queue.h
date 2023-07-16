#pragma once

#include <vector>
#include <mutex>
#include <condition_variable>

template<typename T>
class queue {
private:
    std::vector<T> buffer; //fixed-sized buffer
    mutable std::mutex mutex;
    std::condition_variable pop_cv;
    std::condition_variable push_cv;
    size_t begin;
    size_t count;
    size_t capacity;

    bool full() const {
        return count == capacity;
    }

    bool empty() const {
        return count == 0;
    }

public:
    queue(size_t _capacity) {
        buffer.reserve(_capacity);
        capacity = _capacity;
        begin = 0;
        count = 0;
    }

    void push(T data) {
        std::unique_lock<std::mutex> ul(mutex);
        push_cv.wait(ul, [this]() { return !full(); });
        buffer[(begin + count) % capacity] = std::move(data);
        ++count;
        ul.unlock();
        pop_cv.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> ul(mutex);
        pop_cv.wait(ul, [this]() { return !empty(); });
        T data = std::move(buffer[begin]);
        --count;
        begin = (begin + 1) % capacity;
        ul.unlock();
        push_cv.notify_one();
        return data;
    }

};