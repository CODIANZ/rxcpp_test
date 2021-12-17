#include "test_1.h"

#include <iostream>
#include "../tools/some_api.h"
#include "../tools/unit.h"
#include "../tools/scope_counter.h"
#include "../tools/ready_set_go.h"

/** 繰り返しで retry() を使う */
void test_1_1() {
  auto api = std::make_shared<some_api>(5);

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

/** 繰り返しで never() を使う */
void test_1_2() {
  auto api = std::make_shared<some_api>(5);
  auto sbj = rxcpp::subjects::subject<unit>();

  auto sbsc = ready_set_go([=](){
    sbj.get_subscriber().on_next(unit{});
  }, sbj.get_observable())
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
  .flat_map([=](result x){
    scope_counter sc;
    if(x == result::failure){
      sbj.get_subscriber().on_next(unit{});
      return rxcpp::observable<>::never<unit>().as_dynamic();
    }
    else{
      /**
       * ここで sbj を　complete させるべく
       * sbj.get_subscriber().on_completed();
       * を実行しても、この flat_map が never を返却していると、
       * もはや、全ての observable を complete することが「絶対に不可能」なので、
       * 適宜、後段で take() することで、前段の購読完了の意思表示をしなければ
       * subscribe で complete が呼び出されないので注意。
       **/
      return rxcpp::observable<>::just(unit{}).as_dynamic();
    }
  }).as_dynamic()
  .take(1) /** 明示的な購読終了の意思表示をしないと、全てが complete しないので注意　*/
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
    scope_counter::dump();
  });
  /**
   * ちなみに、 take(1) が無くても、この関数は終了するが
   * その場合、 subscription は終了していない状態で、この関数を抜けるため
   * subscription のデストラクタで unsubscribe されているだけということに注意が必要
   **/

  while(sbsc.is_subscribed()) {}
}

/** 繰り返しで skip_while() を使う */
void test_1_3() {
  auto api = std::make_shared<some_api>(5);
  auto sbj = rxcpp::subjects::subject<unit>();

  auto sbsc = ready_set_go([=](){
    sbj.get_subscriber().on_next(unit{});
  }, sbj.get_observable())
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
  .tap([=](result x){
    if(x == result::failure){
      sbj.get_subscriber().on_next(unit{});
    }
    else{
      sbj.get_subscriber().on_completed();
    }
  })
  .skip_while([=](result x){
    return x == result::failure;
  }).as_dynamic()
  .take(1)
  .subscribe([=](result){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
    scope_counter::dump();
  });

  while(sbsc.is_subscribed()) {}
}

/** 繰り返しで observable を作成する */
void test_1_4() {
  auto api = std::make_shared<some_api>(5);
  auto sbj = rxcpp::subjects::subject<unit>();

  auto sbsc = ready_set_go([=](){
    sbj.get_subscriber().on_next(unit{});
  }, sbj.get_observable())
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
  .flat_map([=](result x){
    return rxcpp::observable<>::create<unit>([=](rxcpp::subscriber<unit> s){
      scope_counter sc;
      if(x == result::failure){
        sbj.get_subscriber().on_next(unit{});
        s.on_completed(); /** 忘れがちなので要注意！ */
      }
      else {
        s.on_next(unit{});
        s.on_completed();
        sbj.get_subscriber().on_completed();
      }
    });
  }).as_dynamic()
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

void test_1_5() {
  rxcpp::observable<>::create<unit>([](rxcpp::subscriber<unit> s){
    try{
      some_api api(5);
      auto results = api.call().as_blocking();
      while(results.first() == result::failure) {}
      s.on_next(unit{});
      s.on_completed();
    }
    catch(std::exception& e){
      s.on_error(std::make_exception_ptr(e));
    }
  })
  .subscribe([=](unit){
    std::cout << "on next" << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
    scope_counter::dump();
  });
}

void test_1() {
  std::cout << std::endl << "test_1_1()" << std::endl;
  test_1_1();
  scope_counter::reset();

  std::cout << std::endl << "test_1_2()" << std::endl;
  test_1_2();
  scope_counter::reset();

  std::cout << std::endl << "test_1_3()" << std::endl;
  test_1_3();
  scope_counter::reset();

  std::cout << std::endl << "test_1_4()" << std::endl;
  test_1_4();
  scope_counter::reset();

  std::cout << std::endl << "test_1_5()" << std::endl;
  test_1_5();
  scope_counter::reset();
}
