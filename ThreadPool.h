
#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <thread>
#include <vector>
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace vk{
class Task {
public:
    template<typename Func, typename ...ARGS>
    Task(Func f, ARGS ...args) {
        func = std::bind(f, std::forward<ARGS>(args)...); 
    }
    void run() {
        func();
    }
private:
    std::function<void()> func;
};


class thread_pool {
public:
    thread_pool(int sz = 5)
    : thread_size(sz), is_started(false){}

    ~thread_pool() {
        stop();
    }

    void start();
    void stop();
    void stop_until_empty();

    template <typename Func, typename ...ARGS>
    void add_task(Func f, ARGS...args);
       

private:
    int thread_size;
    bool is_started;
    std::vector<std::thread *> threads;
    std::queue<Task *> tasks;
    std::mutex m_mutex;
    std::mutex m_queue_mutex;
    std::mutex m_info_mutex;
    std::condition_variable m_cond;
    std::condition_variable m_queue_cond;
    void thread_loop();
    Task *get_task();
    void __add_task(Task *);
};

template <typename Func, typename ...ARGS>
void thread_pool::add_task(Func f, ARGS...args) {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    __add_task(new Task(f, std::forward<ARGS>(args)...));
    return ;
}

void thread_pool::__add_task(Task *t) {
    std::unique_lock<std::mutex> lock(m_mutex);
    tasks.push(t);
    m_cond.notify_one();
    return ;
}

Task *thread_pool::get_task() {
    std::unique_lock<std::mutex> lock1(m_mutex);
    while (tasks.empty() && is_started) {
        m_cond.wait(lock1);
    }
    Task *t = nullptr;
    if (!tasks.empty() && is_started) {
        t = tasks.front();
        tasks.pop();
        if (tasks.empty()) {
            std::unique_lock<std::mutex> lock2(m_info_mutex);
            m_queue_cond.notify_all();
        }
    } 
    return t;
}

void thread_pool::thread_loop() {
    while (is_started) {
        Task *t = get_task();
        if (t != nullptr) {
            t->run();
        }
    }
}

void thread_pool::start() {
    std::unique_lock<std::mutex> lock(m_mutex);
    is_started = true;
    for (int i=0; i < thread_size; ++i) {
        threads.push_back(new std::thread(&thread_pool::thread_loop, this));
    }
}

void thread_pool::stop() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        is_started = false;
        m_cond.notify_all();
    }
    std::cout << thread_size << " " << threads.size() << std::endl;
    for (int i = 0; i < threads.size(); ++i) {
        threads[i]->join();
        delete threads[i];
    }
    threads.clear();
    return ;
}

void thread_pool::stop_until_empty() {
    std::unique_lock<std::mutex> lock1(m_info_mutex);
    std::unique_lock<std::mutex> lock2(m_queue_mutex);
    while (!tasks.empty()) {
        m_queue_cond.wait(lock1);
    }
    stop();
    return ;
}
}

#endif
