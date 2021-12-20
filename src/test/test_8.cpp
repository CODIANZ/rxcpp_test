#include "test_8.h"

#include <iostream>
#include <array>
#include "../tools/inflow_restriction.h"
#include "../tools/some_api.h"

static std::mutex mtx; /* コンソール出力の排他制御用 */

/** 呼び出してから結果を発行するまで１秒かかる非同期 API */
struct long_api {
  int count_ = 0; /** mtx に便乗して保護する */

  rxcpp::observable<result> call() {
    return rxcpp::observable<>::just(unit{}, rxcpp::observe_on_new_thread())
    .tap([=](unit){
      std::lock_guard<std::mutex> lock(mtx);
      const int x = count_++; 
      std::cout << std::this_thread::get_id() << " : enter " << x << std::endl;
    })
    .delay(std::chrono::seconds(1), rxcpp::observe_on_new_thread())
    .map([=](unit){
      return result::success;
    })
    .tap([=](result){
      std::lock_guard<std::mutex> lock(mtx);
      const int x = count_--;
      std::cout << std::this_thread::get_id() << " : leave " << x << std::endl;
    });
  }
};

void test_8_1() {
  auto list = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  auto api = std::make_shared<long_api>();

  auto sbsc = rxcpp::observable<>::iterate(list)
  .flat_map([=](int n){
    return api->call()
    .map([=](result){
      return n;
    });
  })
  .subscribe([=](int x){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on next " << x << std::endl;
  }, [=](std::exception_ptr){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on error" << std::endl;
  }, [=](){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on complete" << std::endl;
  });

  while(sbsc.is_subscribed()) {}
}

void test_8_2() {
  auto list = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  auto api = std::make_shared<long_api>();
  auto ifr = std::make_shared<inflow_restriction<4>>();

  auto sbsc = rxcpp::observable<>::iterate(list)
  .flat_map([=](int n){
    return ifr->enter(api->call())
    .map([=](result){
      return n;
    });
  })
  .subscribe([=](int x){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on next " << x << std::endl;
  }, [=](std::exception_ptr){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on error" << std::endl;
  }, [=](){
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << std::this_thread::get_id() << " : on complete" << std::endl;
  });

  while(sbsc.is_subscribed()) {}
}

void test_8() {
  std::cout << std::endl << "test_8_1()" << std::endl;
  test_8_1();

  std::cout << std::endl << "test_8_2()" << std::endl;
  test_8_2();
}
