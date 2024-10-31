#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template<typename T>
class threadsafe_queue
{
  private:
    mutable std::mutex mtx;                // Mutex to synchronize access
    std::queue<T> data_queue;              // Queue to hold the data
    std::condition_variable data_cond;     // Condition variable to handle blocking

  public:
    threadsafe_queue() {}

    threadsafe_queue(const threadsafe_queue& other){
	    std::lock_guard<std::mutex> lock(other.mtx);
        data_queue = other.data_queue;
    }

    threadsafe_queue& operator=(const threadsafe_queue&) = delete;

    void push(T new_value){
        std::lock_guard<std::mutex> lock(mtx);       // Lock the mutex
        data_queue.push(std::move(new_value));       // Push new element
        data_cond.notify_one();                     // Notify one waiting thread
    }       

    bool try_pop(T& value){
	    std::lock_guard<std::mutex> lock(mtx);       // Lock the mutex
        if (data_queue.empty())                      // Check if queue is empty
            return false;
        value = std::move(data_queue.front());       // Retrieve front item
        data_queue.pop();                            // Remove front item
        return true;
    }

    void wait_and_pop(T& value){
	    std::unique_lock<std::mutex> lock(mtx);      // Lock the mutex with unique_lock for condition_variable
        data_cond.wait(lock, [this]{ return !data_queue.empty(); });  // Wait until data is available
        value = std::move(data_queue.front());       // Retrieve front item
        data_queue.pop();       
    }

    std::shared_ptr<T> wait_and_pop(){
	    std::unique_lock<std::mutex> lock(mtx);      // Lock the mutex
        data_cond.wait(lock, [this]{ return !data_queue.empty(); });  // Wait until data is available
        std::shared_ptr<T> result = std::make_shared<T>(std::move(data_queue.front()));  // Retrieve item as shared_ptr
        data_queue.pop();                            // Remove front item
        return result;
    }

    bool empty() const{
	    std::lock_guard<std::mutex> lock(mtx);       // Lock the mutex
        return data_queue.empty();                   // Check if queue is empty
    }
};
