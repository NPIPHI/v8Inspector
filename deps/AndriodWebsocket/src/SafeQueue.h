//
// Created by 16182 on 12/14/2020.
//

#ifndef V8DEBUGGER_SAFEQUEUE_H
#define V8DEBUGGER_SAFEQUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

// A threadsafe-queue.
template <class T>
class SafeQueue
{
public:
    SafeQueue(): q(), m(), c(){}

    // Add an element to the queue.
    inline void enqueue(T t) {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    inline std::optional<T> dequeue(void) {
        std::unique_lock<std::mutex> lock(m);
        if(q.empty()){
            return std::nullopt;
        } else {
            T val = q.front();
            q.pop();
            return std::move(val);
        }
    }

    T blocking_dequeue(void) {
        std::unique_lock<std::mutex> lock(m);
        while(q.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }
        T val = q.front();
        q.pop();
        return std::move(val);
    }
    inline size_t size(){
        std::unique_lock<std::mutex> lock(m);
        return q.size();
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};


#endif //V8DEBUGGER_SAFEQUEUE_H
