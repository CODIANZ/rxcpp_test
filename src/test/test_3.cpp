#include "test_3.h"

#include <iostream>
#include "../tools/some_state.h"
#include "../tools/unit.h"
#include "../tools/scope_counter.h"
#include "../tools/ready_set_go.h"

/** 引き金を引いてから仕掛けるパターン（取りこぼしする） */
void test_3_1() {
  auto fn = [](std::shared_ptr<some_state> state){
    state->start();
    return state->observable();
  };

  auto state = std::make_shared<some_state>();

  auto sbsc = fn(state)
  .take(5)
  .subscribe([=](int n){
    std::cout << n << std::endl;
  }, [](std::exception_ptr){
  }, [=](){
  });
  state->end();

  while(sbsc.is_subscribed()) {}
}

/** 仕掛けてから引き金を引いくパターン（取りこぼさない） */
void test_3_2() {
  auto fn = [](std::shared_ptr<some_state> state){
    return ready_set_go([state](){
      state->start();
    }, state->observable());
  };

  auto state = std::make_shared<some_state>();

  auto sbsc = fn(state)
  .take(5)
  .subscribe([=](int n){
    std::cout << n << std::endl;
  }, [](std::exception_ptr){
  }, [=](){
  });
  state->end();

  while(sbsc.is_subscribed()) {}
}


void test_3() {
  std::cout << std::endl << "test_3_1()" << std::endl;
  test_3_1();
  scope_counter::reset();

  std::cout << std::endl << "test_3_2()" << std::endl;
  test_3_2();
  scope_counter::reset();
} 