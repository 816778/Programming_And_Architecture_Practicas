#pragma once

#include <atomic>
#include <functional>
#include <vector>

#include<join_threads.hpp>
#include<threadsafe_queue.hpp>

class thread_pool
{
  using task_type = void();

private:
    std::atomic<bool> _done;                                 // Flag to stop all threads
    threadsafe_queue<std::function<void()>> _work_queue;     // Queue of tasks
    std::vector<std::thread> _threads; 
    join_threads _joiner;

    void worker_thread(){
      while (!_done){
          std::function<void()> task;
          if (_work_queue.try_pop(task)){
              task();  
          }
          else{
              std::this_thread::yield(); 
          }
      }
    }
  

  public:
    thread_pool(size_t num_threads = std::thread::hardware_concurrency()) : _done(false), _joiner(_threads){
        for (size_t i = 0; i < num_threads; ++i){
          _threads.emplace_back(&thread_pool::worker_thread, this);  // Start worker threads
        }

  }

  ~thread_pool(){
    wait();
  }

  void wait(){
    _done = true;  // Signal all threads to stop
    for (auto& thread : _threads){
        if (thread.joinable()){
            thread.join();  // Join each thread
        }
    }
  }

  template<typename F>void submit(F f){
	  _work_queue.push(std::function<void()>(f));  // Wrap and push the task into the queue
  }
};
