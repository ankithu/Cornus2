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

    T waitForNextMessage(std::chrono::duration<double> timeout){
        //blocks until message is available
        auto lock = std::unique_lock(mutex);
        if (!q.empty()){
            return popFront();
        }
        //TODO: change this to a wait until timeout
        cv.wait(lock, [this]{return !q.empty();});
        return popFront();
    }

    void setTimeoutStart(){
        auto lock = std::unique_lock(mutex);
        //set timeoutStart
        //timeoutStart = std::chrono::steady_clock::now();
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
    std::chrono::time_point<std::chrono::system_clock, >()> timeoutStart;
};

#endif //CORNUS_MESSAGEQUEUE_HPP
