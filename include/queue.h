//https://stackoverflow.com/questions/15278343/c11-thread-safe-queue
#ifndef SAFE_QUEUE
#define SAFE_QUEUE

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

// A threadsafe-queue.
template <class T>
class Queue
{
public:
  Queue(void)
    : q()
    , m()
    , c()
  {}

  ~Queue(void)
  {}

  // Add an element to the queue.
  void put(T t)
  {
    std::lock_guard<std::mutex> lock(m);
    q.push(t);
    c.notify_one();
  }

  // Get the "front"-element.
  // If the queue is empty, wait till a element is avaiable.
  T get_or_wait(void)
  {
    std::unique_lock<std::mutex> lock(m);
    while(q.empty())
    {
      // release lock as long as the wait and reaquire it afterwards.
      c.wait(lock);
    }
    T val = q.front();
    q.pop();
    return val;
  }

  std::optional<T> get(void)
  {
    std::unique_lock<std::mutex> lock(m);
    if(q.empty()) {
        return {};
    }
    T val = q.front();
    q.pop();
    return val;
  }

private:
  std::queue<T> q;
  mutable std::mutex m;
  std::condition_variable c;
};
#endif
