/*
 * UnRAWer - camera raw bnatch processor on top of OpenImageIO
 * Copyright (c) 2023 Erium Vladlen.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#ifndef _UNRAWER_THREADPOOL_HPP
#define _UNRAWER_THREADPOOL_HPP

#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <future>
#include <stdexcept>
#include <thread>

//template <typename T>
//class SafeQueue {
//private:
//    std::queue<T> queue;
//    std::mutex mtx;
//    std::condition_variable cv;
//
//public:
//    T pop() {
//        std::unique_lock<std::mutex> lock(mtx);
//        while (queue.empty()) {
//            cv.wait(lock);
//        }
//        auto item = queue.front();
//        queue.pop();
//        return item;
//    }
//
//    void push(const T& item) {
//        std::unique_lock<std::mutex> lock(mtx);
//        queue.push(item);
//        lock.unlock();
//        cv.notify_one();
//    }
//
//    std::optional<T> try_pop() {
//        std::unique_lock<std::mutex> lock(mtx);
//        if (queue.empty()) {
//            return {};
//        }
//        auto item = queue.front();
//        queue.pop();
//        return item;
//    }
//

//};
template <typename T>
class SafeQueue {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    int maxSize;

public:
    SafeQueue() : maxSize(-1) {}

    SafeQueue(int maxSize) : maxSize(maxSize) {}

    T pop() {
        std::unique_lock<std::mutex> lock(mtx);
        while (queue.empty()) {
            cv.wait(lock);
        }
        auto item = queue.front();
        queue.pop();
        return item;
    }

    void push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        while (maxSize != -1 && queue.size() >= maxSize) {
            cv.wait(lock);
        }
        queue.push(item);
        lock.unlock();
        cv.notify_all();
    }

    std::optional<T> try_pop() {
        std::unique_lock<std::mutex> lock(mtx);
        if (queue.empty()) {
            return {};
        }
        auto item = queue.front();
        queue.pop();
        return item;
    }

    bool try_push(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        if (maxSize != -1 && queue.size() >= maxSize) {
            return false;
        }
        queue.push(item);
        lock.unlock();
        cv.notify_all();
        return true;
    }


    bool isEmpty() {
        std::unique_lock<std::mutex> lock(mtx);
        return queue.empty();
    }
};

class ThreadPool {
public:
    ThreadPool(size_t threads, size_t maxQueueSize)
        : stop(false),
        working(0),
        maxQueueSize(maxQueueSize),
        tasks_count(0) {
        // Create worker threads
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        // Wait until there is a task or the ThreadPool is stopped
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                            });
                        if (this->stop && this->tasks.empty()) {
                            return; // Exit the thread when ThreadPool is stopped
                        }
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                        ++this->working;
                    }
                    task(); // Execute the task
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        --this->working;
                        --this->tasks_count;
                        this->done_condition.notify_all();
                        if (this->tasks_count < this->maxQueueSize) {
                            this->queue_full = false;
                        }
                    }
                }
            });
        }
    }

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            while (tasks_count >= maxQueueSize) {
                // Wait until there is space in the task queue
                condition.wait_for(lock, std::chrono::milliseconds(100));
            }

            tasks.emplace([task]() { (*task)(); });
            ++this->tasks_count;
            if (this->tasks_count >= this->maxQueueSize) {
                this->queue_full = true;
            }
        }
        condition.notify_one();
        return res;
    }

    bool isIdle() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        return tasks.empty() && (working == 0);
    }

    void waitForAllTasks() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        done_condition.wait(lock, [this] { return tasks_count == 0; });
    }

    void setWritePoolLimitation(size_t limit) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        maxQueueSize = limit;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }

private:
    std::vector<std::thread> workers;                // Worker threads
    std::queue<std::function<void()>> tasks;         // Task queue
    std::mutex queue_mutex;                          // Mutex to protect the task queue
    std::condition_variable condition;               // Condition variable for task availability
    std::condition_variable done_condition;          // Condition variable for completion of all tasks
    std::atomic<bool> stop;                          // Atomic flag to stop the ThreadPool
    std::atomic<int> working;                        // Atomic counter for the number of working threads
    std::atomic<int> tasks_count;                    // Atomic counter for the number of tasks in the queue
    size_t maxQueueSize;                             // Maximum size of the task queue
    std::atomic<bool> queue_full = false;            // Atomic flag indicating if the task queue is full
};

//class ThreadPool {
//public:
//    ThreadPool(size_t threads) : stop(false), working(0), tasks_count(0) {
//        for (size_t i = 0; i < threads; ++i)
//            workers.emplace_back([this] {
//            for (;;) {
//                std::function<void()> task;
//                {
//                    std::unique_lock<std::mutex> lock(this->queue_mutex);
//                    this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
//                    if (this->stop && this->tasks.empty())
//                        return;
//                    task = std::move(this->tasks.front());
//                    this->tasks.pop();
//                    ++this->working;
//                }
//                task();
//                {
//                    std::unique_lock<std::mutex> lock(this->queue_mutex);
//                    --this->working;
//                    --this->tasks_count;
//                    this->done_condition.notify_all();
//                }
//            }
//                });
//    }
//
//    template<class F, class... Args>
//    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
//    {
//        using return_type = typename std::result_of<F(Args...)>::type;
//
//        auto task = std::make_shared< std::packaged_task<return_type()> >(
//            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
//        );
//
//        std::future<return_type> res = task->get_future();
//        {
//            std::unique_lock<std::mutex> lock(queue_mutex);
//
//            // don't allow enqueueing after stopping the pool
//            if (stop)
//                throw std::runtime_error("enqueue on stopped ThreadPool");
//
//            tasks.emplace([task]() { (*task)(); });
//            ++this->tasks_count;
//        }
//        condition.notify_one();
//        return res;
//    }
//
//    bool isIdle() {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        return tasks.empty() && (working == 0);
//    }
//
//    void waitForAllTasks() {
//        std::unique_lock<std::mutex> lock(queue_mutex);
//        done_condition.wait(lock, [this] { return tasks_count == 0; });
//    }
//
//    ~ThreadPool() {
//        {
//            std::unique_lock<std::mutex> lock(queue_mutex);
//            stop = true;
//        }
//        condition.notify_all();
//        for (std::thread& worker : workers)
//            worker.join();
//    }
//
//private:
//    std::vector<std::thread> workers;
//    std::queue<std::function<void()>> tasks;
//    std::mutex queue_mutex;
//    std::condition_variable condition;
//    std::condition_variable done_condition;
//    std::atomic<bool> stop;
//    std::atomic<int> working;
//    std::atomic<int> tasks_count;
//};

#endif // !_UNRAWER_THREADPOOL_HPP
