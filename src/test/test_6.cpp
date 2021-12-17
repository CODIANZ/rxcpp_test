#include "test_6.h"

#include <iostream>
#include "../tools/unit.h"
#include "../tools/debug_scheduler.h"

rxcpp::observable<unit> obs(std::string name = std::string()) {
  return rxcpp::observable<>::create<unit>([name](rxcpp::subscriber<unit> s){
    std::cout << name << "emit next" << " {" << std::endl;
    s.on_next(unit{});
    std::cout << name << "emit next" << " }" << std::endl;
    std::cout << name << "emit complete" << " {" << std::endl;
    s.on_completed();
    std::cout << name << "emit complete" << " }" << std::endl;
  });
}

void test_6_1()
{
  obs()
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}


void test_6_2(){
  obs()
  .observe_on(observe_on_debug("observe_on"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_3(){
  obs()
  .subscribe_on(observe_on_debug("subscribe_on"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_4(){
  rxcpp::observable<>::just(unit{}, observe_on_debug("just"))
  .flat_map([](unit){
    return obs();
  })
  .observe_on(observe_on_debug("observe_on"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_5(){
  rxcpp::observable<>::just(unit{}, observe_on_debug("just"))
  .flat_map([](unit){
    return obs();
  })
  .subscribe_on(observe_on_debug("subscribe_on"))
  .subscribe(
    [=](unit){ std::cout << "on next" << std::endl; },
    [=](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [=](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_6(){
  rxcpp::observable<>::just(unit{}, observe_on_debug("just"))
  .observe_on(observe_on_debug("observe_on #1"))
  .flat_map([](unit){
    return obs();
  })
  .observe_on(observe_on_debug("observe_on #2"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_7(){
  rxcpp::observable<>::just(unit{}, observe_on_debug("just"))
  .observe_on(observe_on_debug("observe_on"))
  .flat_map([](unit){
    return obs();
  })
  .subscribe_on(observe_on_debug("subscribe_on"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_8(){
  obs()
  .subscribe_on(observe_on_debug("subscribe_on #1"))
  .observe_on(observe_on_debug("observe_on"))
  .subscribe_on(observe_on_debug("subscribe_on #2"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_9(){
  obs("(A) ")
  .flat_map([](unit){
    return obs("(B) ")
    .subscribe_on(observe_on_debug("subscribe_on #1"));
  })
  .subscribe_on(observe_on_debug("subscribe_on #2"))
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}

void test_6_10(){
  rxcpp::observable<>::just(unit{}, observe_on_debug("observe_on #2"))
  .flat_map([](unit){
    return obs("(A) ");
  })
  .flat_map([](unit){
    return rxcpp::observable<>::just(unit{}, observe_on_debug("observe_on #1"))
    .flat_map([](unit){
      return obs("(B) ");
    });
  })
  .subscribe(
    [](unit){ std::cout << "on next" << std::endl; },
    [](std::exception_ptr){ std::cout << "on error" << std::endl; },
    [](){ std::cout << "on complete" << std::endl; }
  );
}


void test_6() {
  std::cout << std::endl << "test_6_1()" << std::endl;
  test_6_1();

  std::cout << std::endl << "test_6_2()" << std::endl;
  test_6_2();

  std::cout << std::endl << "test_6_3()" << std::endl;
  test_6_3();

  std::cout << std::endl << "test_6_4()" << std::endl;
  test_6_4();

  std::cout << std::endl << "test_6_5()" << std::endl;
  test_6_5();

  std::cout << std::endl << "test_6_6()" << std::endl;
  test_6_6();

  std::cout << std::endl << "test_6_7()" << std::endl;
  test_6_7();

  std::cout << std::endl << "test_6_8()" << std::endl;
  test_6_8();

  std::cout << std::endl << "test_6_9()" << std::endl;
  test_6_9();

  std::cout << std::endl << "test_6_10()" << std::endl;
  test_6_10();
}
