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

    std::mutex _mutex;                                       // Mutex for condition variable
    std::condition_variable _cv;                             // Condition variable to signal task completion
    std::atomic<int> _active_tasks;                          // Counter for active tasks

    /**
     * Continuously tries to pop tasks from _work_queue and execute them.
     * If the queue is empty, it calls std::this_thread::yield() to indicate to the OS that it’s idle,
     * allowing other threads to use the CPU.
     */
    void worker_thread(){
      while (!_done || !_work_queue.empty()){
          std::function<void()> task;
          if (_work_queue.try_pop(task)){
              ++_active_tasks;                             // Increment active task count
              task();                                      // Execute the task
              --_active_tasks;  
              // Notify wait() if all tasks are completed
              if (_active_tasks == 0 && _work_queue.empty()){
                  std::lock_guard<std::mutex> lock(_mutex);
                  _cv.notify_all();                        // Notify that all tasks are done
              }
          }
          else{
              std::this_thread::yield(); 
          }
      }
    }
  

  public:
    thread_pool(size_t num_threads = std::thread::hardware_concurrency()) 
    : _done(false), _joiner(_threads), _active_tasks(0) {
        for (size_t i = 0; i < num_threads; ++i){
          _threads.emplace_back(&thread_pool::worker_thread, this);  // Start worker threads
        }

  }

  ~thread_pool(){
    wait();
  }

  /**
   * Ensures that all threads finish their tasks and join the thread_pool before
   * exiting. The wait function sets the _done flag to true, causing each thread
   * in the thread_pool to exit its work loop.
  */
  void wait(){
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this] { return _active_tasks == 0 && _work_queue.empty(); });
    _done = true;  // Signal all threads to stop
    for (auto& thread : _threads){
        if (thread.joinable()){
            thread.join();
        }
    }
  }

  /**
   * Allows adding new tasks to the _work_queue. Tasks are stored in the queue
   * to be executed by the thread_pool threads.
  */
  template<typename F>void submit(F f){
	  _work_queue.push(std::function<void()>(f));  // Wrap and push the task into the queue
    _cv.notify_all();
  }
};
