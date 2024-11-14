#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include <chrono>
#include <thread>
#include<join_threads.hpp>
#include<threadsafe_queue.hpp>

class thread_pool
{
  join_threads _joiner;
  using task_type = void();

  std::atomic<bool> _done; 
  threadsafe_queue<task_type> _work_queue; 
  std::vector<std::thread> _threads;  

  void worker_thread() {
    while (!_done) {
        task_type task;
        if (_work_queue.try_pop(task)) {
            task(); // Execute the task
        } else {
            std::this_thread::yield(); // Yield if no task is available
        }
    }
  }

  public:
  thread_pool(size_t num_threads = std::thread::hardware_concurrency())
    :  _done(false), _joiner(_threads)
  {
     for (size_t i = 0; i < num_threads; ++i) {
          _threads.emplace_back(&thread_pool::worker_thread, this); // Start each worker thread
      }
  }

  ~thread_pool()
  {
    _done = true;
    wait();
  }

  void wait()
  {
      while (!_work_queue.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for 10ms
      }
  }

  template<typename F>
    void submit(F f)
    {
      _work_queue.push(std::function<void()>(f));
    }
};