#if !defined(__h_sem__)
#define __h_sem__

#include <mutex>
#include <condition_variable>

template <int N = 1> class sem {
static_assert(N > 0, "the count max in a semaphore must be grater than 1");
private:
  static constexpr int    count_max_ = N;
  volatile int            count_;
  std::mutex              mtx_;
  std::condition_variable cond_;

public:
  explicit sem(int initial_count = count_max_) : count_(initial_count) {}
  sem(const sem&) = delete; /** non copyable */
  ~sem() = default;

  /** BasicLockable */
  void lock() { 
    std::unique_lock<std::mutex> lock(mtx_);
    cond_.wait(lock, [&]{
      return count_ > 0;
    });
    count_--;    
  }

  void unlock() { 
    std::lock_guard<std::mutex> lock(mtx_);
    if (count_ < count_max_) {
      count_++;
      cond_.notify_one();
    }
  }

  /** Lockable */
  bool try_lock() {
    std::lock_guard<std::mutex> lock(mtx_);
    if(count_ > 0) {
      count_--;
      return true;
    }
    else {
      return false;
    }
  }
};

#endif /* !defined(__h_sem__) */