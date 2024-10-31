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

    /**
     * continuously tries to pop tasks from _work_queue and execute them.
     * If the queue is empty, it calls std::this_thread::yield() to indicate to the OS that it’s idle,
     * allowing other threads to use the CPU.
     */
    void worker_thread(){
      while (!_done || !_work_queue.empty()){
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

  /**
   * Se asegura de que todos los hilos terminen sus tareas y se "unen" 
   * (join) al thread_pool antes de finalizar. La función wait establece 
   * la bandera _done en true, lo que hace que cada hilo en el thread_pool 
   * finalice su bucle de trabajo. 
   */
  void wait(){
    _done = true;  // Signal all threads to stop
    for (auto& thread : _threads){
        if (thread.joinable()){
            thread.join();  // Join each thread
        }
    }
  }

  /**
   * Permite agregar nuevas tareas a la cola de trabajo _work_queue. Las tareas se almacenan 
   * en la cola para ser ejecutadas por los hilos del thread_pool
   */
  template<typename F>void submit(F f){
	  _work_queue.push(std::function<void()>(f));  // Wrap and push the task into the queue
  }
};
