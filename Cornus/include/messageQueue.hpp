#ifndef CORNUS_MESSAGEQUEUE_HPP
#define CORNUS_MESSAGEQUEUE_HPP

#include <queue>
#include <condition_variable>
#include <mutex>

//A thread safe generic message queue
template <typename T>
struct MessageQueue {
public:
    void push(T element){
        auto lock = std::unique_lock(mutex);
        q.push(element);
        cv.notify_one();
    }

    T waitForNextMessage(){
        //blocks until message is available
        auto lock = std::unique_lock(mutex);
        if (!q.empty()){
            return popFront();
        }
        cv.wait(lock, [this]{return !q.empty();});
        return popFront();
    }

private:

    T popFront(){
        //assumes exclusive ownership of q
        T e = q.front();
        q.pop();
        return e;
    }

    std::queue<T> q;
    std::condition_variable cv;
    std::mutex mutex;
};

#endif //CORNUS_MESSAGEQUEUE_HPP
