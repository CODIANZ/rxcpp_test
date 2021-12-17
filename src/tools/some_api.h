#if !defined(__h_some_api__)
#define __h_some_api__

#include <rxcpp/rx.hpp>
#include "unit.h"

/* some_api を call() した時の結果 */
enum class result { success, failure };

/** 
 * n - 1 回目 まで failure を返却
 * n 回目で success を返却
 * n + 1 回目以降は failure を返却
 **/
class some_api {
private:
  std::atomic_int n_;
public:
  some_api(int n) : n_(n) {}
  ~some_api() = default;
  auto call() -> rxcpp::observable<result>
  {
    return rxcpp::observable<>::just(unit{}, rxcpp::observe_on_new_thread())
    .map([&](unit){
      const int n = --n_;
      std::cout << "fire: " << n << std::endl;
      return n == 0 ? result::success : result::failure;
    });
  }
};

/* call() が同期実行されるケース */
class some_api_sync {
private:
  std::atomic_int n_;
public:
  some_api_sync(int n) : n_(n) {}
  ~some_api_sync() = default;
  auto call() -> rxcpp::observable<result>
  {
    return rxcpp::observable<>::just(unit{})
    .map([&](unit){
      const int n = --n_;
      std::cout << "fire: " << n << std::endl;
      return n == 0 ? result::success : result::failure;
    });
  }
};

#endif /* !defined(__h_some_api__) */