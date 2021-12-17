#include "test_2.h"

#include <iostream>
#include "../tools/some_api.h"
#include "../tools/unit.h"
#include "../tools/scope_counter.h"
#include "../tools/async_scheduler.h"

/** 再帰が深すぎてクラッシュする */
void test_2_1() {
  auto api = std::make_shared<some_api_sync>(1000);

  auto sbsc = api->call()
  .map([=](result x){
    scope_counter sc;
    if(x == result::failure) throw 0;
    return unit{};
  }).as_dynamic()
  .retry()
  .take(1)
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
    scope_counter::dump();
  });

  while(sbsc.is_subscribed()) {}
}

/** 再帰を抑止する */
void test_2_2() {
  auto api = std::make_shared<some_api_sync>(1000);

  auto sbsc = api->call()
  .observe_on(observe_on_async())
  .map([=](result x){
    scope_counter sc;
    if(x == result::failure) throw 0;
    return unit{};
  }).as_dynamic()
  .retry()
  .take(1)
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
    scope_counter::dump();
  });

  while(sbsc.is_subscribed()) {}
}


void test_2() {
  /** test_2_1() は実行するとクラッシュします */
  // std::cout << std::endl << "test_2_1()" << std::endl;
  // test_2_1();
  // scope_counter::reset();

  std::cout << std::endl << "test_2_2()" << std::endl;
  test_2_2();
  scope_counter::reset();
} 