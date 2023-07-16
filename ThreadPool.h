#pragma once

#include <vector>
#include <future>
#include <thread>


template<template<class> class Thread_safe_queue>
class ThreadPool {
private:
    struct ITask {
        virtual void f() = 0;

        ~ITask() = default;
    };

    template<class T>
    struct Task : public ITask {
        std::packaged_task<T> task;

        Task(std::packaged_task<T> &&pt) : task(std::move(pt)) {};

        void f() override{
            task();
        }
    };

    using TaskPtr = std::unique_ptr<ITask>;
    std::vector<std::thread> buffer;
    Thread_safe_queue<TaskPtr> queue;
public:
    ThreadPool(size_t count_threads, size_t queue_capacity) : queue(queue_capacity) {
        buffer.reserve(count_threads);
        for (size_t i = 0; i < count_threads; ++i) {
            buffer.template emplace_back( [this]() {
                while (true) {
                    queue.pop()->f();
                }
            });
        }
        for(size_t i = 0;i<count_threads;++i){
            buffer[i].detach();
        }
    };

    template<class T>
    void addTask(std::packaged_task<T> pt){
        TaskPtr newTask = std::make_unique<Task<T>>(std::move(pt));
        queue.push(std::move(newTask));
    }
};
