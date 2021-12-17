#include "test_4.h"

#include <iostream>
#include <sstream>
#include "../tools/some_api.h"
#include "../tools/unit.h"
#include "../tools/scope.h"

void test_4_1() {
  int counter = 0;  /** fn() 内の observable で活躍するカウンタ */

  /**
   * subscribe 毎にインクリメントされた値を１個だけ発行する
   * cold observable を返却する関数
   **/
  auto fn = [&counter]() {
    scope fns("fn()");
    return rxcpp::observable<>::create<int>([&counter](rxcpp::subscriber<int> s){
      scope fnos("observable 1st", counter);
      s.on_next(counter++); /** counter を加算して発行 */
      s.on_completed();
    })
    .flat_map([](int n){
      scope fns("flat_map");
      return rxcpp::observable<>::create<int>([n](rxcpp::subscriber<int> s){
        scope fnos("observable 2nd", n);
        s.on_next(n); /** 受け取った数値（counter） をそのまま発行 */
        s.on_completed();
      });
    });
  };

  fn()
  .map([=](int x){
    std::cout << x << std::endl;
    if(x == 2) return unit{}; /** その後 take(1) で終了 */
    std::cout << "throw" << std::endl;
    throw 0;  /** retry を誘発 */
  })
  .retry()
  .take(1)
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
  });
}

void test_4_2() {
  int counter = 0;  /** fn() 内の observable で活躍するカウンタ */

  /**
   * subscribe 毎にインクリメントされた値を１個だけ発行する
   * cold observable を返却する関数
   **/
  auto fn = [&counter]() {
    scope fns("fn()");
    return rxcpp::observable<>::create<int>([&counter](rxcpp::subscriber<int> s){
      scope fnos("observable 1st", counter);
      s.on_next(counter++); /** counter を加算して発行 */
      s.on_completed();
    })
    .flat_map([](int n){
      scope fns("flat_map");
      return rxcpp::observable<>::create<int>([n](rxcpp::subscriber<int> s){
        scope fnos("observable 2nd", n);
        s.on_next(n); /** 受け取った数値（counter） をそのまま発行 */
        s.on_completed();
      });
    });
  };

  auto sbsc = fn()
  .observe_on(rxcpp::observe_on_new_thread())
  .map([=](int x){
    std::cout << x << std::endl;
    if(x == 2) return unit{}; /** その後 take(1) で終了 */
    std::cout << "throw" << std::endl;
    throw 0;  /** retry を誘発 */
  })
  .retry()
  .take(1)
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
  });

  while(sbsc.is_subscribed()) {}
}

void test_4() {
  std::cout << std::endl << "test_4_1()" << std::endl;
  test_4_1();

  std::cout << std::endl << "test_4_2()" << std::endl;
  test_4_2();
}
