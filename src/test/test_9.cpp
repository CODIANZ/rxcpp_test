#include "test_9.h"

#include <iostream>
#include "../tools/some_api.h"
#include "../tools/unit.h"
#include "../tools/scope_counter.h"
#include "../tools/ready_set_go.h"

struct A {
    A()             { std::cout << "A::ctor default" << std::endl; }
    A(const A&)     { std::cout << "A::ctor copy" << std::endl; }
    A(A&&)          { std::cout << "A::ctor move" << std::endl; }
    ~A()            { std::cout << "A::dtor" << std::endl; }
    void f() const  { std::cout << "A::f" << std::endl; }
};

void test_9_1() {
  auto o = rxcpp::observable<>::just(A());
  {
    auto sbsc = o
    .subscribe([=](auto&& a){
      std::cout << "on next" << std::endl;
    }, [=](std::exception_ptr){
      std::cout << "on error" << std::endl;
    }, [=](){
      std::cout << "on complete" << std::endl;
      scope_counter::dump();
    });
    while(sbsc.is_subscribed()) {}
  }

  {
    auto sbsc = o
    .subscribe([=](auto&& a){
      std::cout << "on next" << std::endl;
    }, [=](std::exception_ptr){
      std::cout << "on error" << std::endl;
    }, [=](){
      std::cout << "on complete" << std::endl;
      scope_counter::dump();
    });
    while(sbsc.is_subscribed()) {}
  }
}


void test_9() {
  std::cout << std::endl << "test_9_1()" << std::endl;
  test_9_1();
  scope_counter::reset();

}
