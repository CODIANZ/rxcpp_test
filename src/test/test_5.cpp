#include "test_5.h"

#include <iostream>
#include "../tools/some_api.h"
#include "../tools/unit.h"
#include "../tools/sem.h"

/** take(1) 通過後に上流でエラーが発生した場合 */
void test_5_1()
{
  auto sbsc = rxcpp::observable<>::interval(std::chrono::milliseconds(100), rxcpp::observe_on_new_thread())
  .observe_on(rxcpp::observe_on_new_thread())
  .flat_map([=](int x){
    std::cout << x << ": enter" << std::endl;
    if(x == 1){
      return rxcpp::observable<>::just(unit{})
      .observe_on(rxcpp::observe_on_new_thread())
      .flat_map([=](unit){
        std::cout << x << ": wait for 1000ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << x << ": throw" << std::endl;
        return rxcpp::observable<>::error<int>(std::make_exception_ptr(std::exception()));
      }).as_dynamic();
    }
    std::cout << x << ": emit" << std::endl;
    return rxcpp::observable<>::just(x).as_dynamic();
  })
  .take(1)
  .observe_on(rxcpp::observe_on_new_thread())
  .map([=](int x){
    std::cout << x << ": after take(1) wait for 2000ms" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    std::cout << x << ": after take(1) emit" << std::endl;
    return x;
  })
  .subscribe([=](int x){
    std::cout << "on next " << x << std::endl;
  }, [=](std::exception_ptr){
    std::cout << "on error" << std::endl;
  }, [=](){
    std::cout << "on complete" << std::endl;
  });
  
  while(sbsc.is_subscribed()) {}
}

/** エラーが先行した場合 */
void test_5_2()
{
  auto sbsc = rxcpp::observable<>::interval(std::chrono::milliseconds(100), rxcpp::observe_on_new_thread())
  .observe_on(rxcpp::observe_on_new_thread())
  .flat_map([=](int x){
    std::cout << std::this_thread::get_id() << " : " << x << " : enter" << std::endl;
    if(x == 1){ 
      return rxcpp::observable<>::just(unit{})
      .observe_on(rxcpp::observe_on_new_thread())
      .flat_map([=](unit){
        std::cout << std::this_thread::get_id() << " : " << x << " : wait for 50ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << std::this_thread::get_id() << " : " << x << " : throw" << std::endl;
        return rxcpp::observable<>::error<int>(std::make_exception_ptr(std::exception()));
      }).as_dynamic();
    }
    std::cout << std::this_thread::get_id() << " : " << x << " : emit" << std::endl;
    return rxcpp::observable<>::just(x).as_dynamic();
  })
  .take(1)
  .observe_on(rxcpp::observe_on_new_thread())
  .on_error_resume_next([=](std::exception_ptr e){
    std::cout << std::this_thread::get_id() << " : on_error_resume_next" << std::endl;
    return rxcpp::observable<>::just(unit{})
    .observe_on(rxcpp::observe_on_new_thread())
    .flat_map([=](unit){
      std::cout << std::this_thread::get_id() << " : on_error_resume_next: wait for 1000ms" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::cout << std::this_thread::get_id() << " : on_error_resume_next: emit" << std::endl;
      return rxcpp::observable<>::just(1);
    });
  })
  .subscribe([=](int x){
    std::cout << std::this_thread::get_id() << " : on next " << x << std::endl;
  }, [=](std::exception_ptr){
    std::cout << std::this_thread::get_id() << " : on error" << std::endl;
  }, [=](){
    std::cout << std::this_thread::get_id() << " : on complete" << std::endl;
  });
  
  while(sbsc.is_subscribed()) {}
}

/** ほぼ同時を狙う */
void test_5_3()
{
  for(auto i = 1;; i++){
    std::cout << std::endl << "========== " << i << "==========" << std::endl; 
    auto count = std::make_shared<std::atomic_int>(0);
    auto mtx = std::make_shared<sem<1>>();
    auto sbsc = rxcpp::observable<>::interval(std::chrono::milliseconds(100), rxcpp::observe_on_new_thread())
    .observe_on(rxcpp::observe_on_new_thread())
    .flat_map([=](int x){
      std::cout << std::this_thread::get_id() << " : " << x << " : enter" << std::endl;
      if(x == 1){ 
        mtx->lock();
        return rxcpp::observable<>::just(unit{})
        .observe_on(rxcpp::observe_on_new_thread())
        .flat_map([=](unit){
          std::cout << std::this_thread::get_id() << " : " << x << " : wait for get mutex" << std::endl;
          mtx->lock();
          std::cout << std::this_thread::get_id() << " : " << x << " : throw" << std::endl;
          return rxcpp::observable<>::error<int>(std::make_exception_ptr(std::exception()));
        }).as_dynamic();
      }
      std::cout << std::this_thread::get_id() << " : " << x << " : unlock mutex" << std::endl;
      mtx->unlock();
      std::this_thread::yield();
      std::cout << std::this_thread::get_id() << " : " << x << " : emit" << std::endl;
      return rxcpp::observable<>::just(x).as_dynamic();
    })
    // .observe_on(rxcpp::observe_on_new_thread()) <-- これを入れれば問題解決
    .take(1)
    .observe_on(rxcpp::observe_on_new_thread())
    .on_error_resume_next([=](std::exception_ptr e){
      std::cout << std::this_thread::get_id() << " : on_error_resume_next" << std::endl;
      return rxcpp::observable<>::just(unit{})
      .observe_on(rxcpp::observe_on_new_thread())
      .flat_map([=](unit){
        std::cout << std::this_thread::get_id() << " : on_error_resume_next: emit" << std::endl;
        return rxcpp::observable<>::just(1);
      });
    })
    .subscribe([=](int x){
      (*count)++;
      std::cout << std::this_thread::get_id() << " : on next " << x << std::endl;
    }, [=](std::exception_ptr){
      std::cout << std::this_thread::get_id() << " : on error" << std::endl;
    }, [=](){
      std::cout << std::this_thread::get_id() << " : on complete" << std::endl;
    });
    
    while(sbsc.is_subscribed()) {}

    std::cout << "----------> next count = " << *count << std::endl;
    if(*count >= 2) break;
  }
}


/** ほぼ同時を狙う */
void test_5_4()
{
  for(auto i = 1;; i++){
    std::cout << std::endl << "========== " << i << "==========" << std::endl; 
    auto count = std::make_shared<std::atomic_int>(0);
    auto mtx = std::make_shared<std::mutex>();
    auto sbsc = rxcpp::observable<>::interval(std::chrono::milliseconds(100), rxcpp::observe_on_new_thread())
    .observe_on(rxcpp::observe_on_new_thread())
    .flat_map([=](int x){
      std::cout << std::this_thread::get_id() << " : " << x << " : enter" << std::endl;
      if(x == 1){ 
        mtx->lock();
        return rxcpp::observable<>::just(unit{})
        .observe_on(rxcpp::observe_on_new_thread())
        .flat_map([=](unit){
          std::cout << std::this_thread::get_id() << " : " << x << " : wait for get mutex" << std::endl;
          mtx->lock();
          std::cout << std::this_thread::get_id() << " : " << x << " : emit" << std::endl;
          return rxcpp::observable<>::just(x).as_dynamic();
        }).as_dynamic();
      }
      std::cout << std::this_thread::get_id() << " : " << x << " : unlock mutex" << std::endl;
      mtx->unlock();
      std::this_thread::yield();
      std::cout << std::this_thread::get_id() << " : " << x << " : emit" << std::endl;
      return rxcpp::observable<>::just(x).as_dynamic();
    })
    // .observe_on(rxcpp::observe_on_new_thread()) <-- これを入れれば問題解決
    .take(1)
    .subscribe([=](int x){
      (*count)++;
      std::cout << std::this_thread::get_id() << " : on next " << x << std::endl;
    }, [=](std::exception_ptr){
      std::cout << std::this_thread::get_id() << " : on error" << std::endl;
    }, [=](){
      std::cout << std::this_thread::get_id() << " : on complete" << std::endl;
    });
    
    while(sbsc.is_subscribed()) {}

    std::cout << "----------> next count = " << *count << std::endl;
    if(*count >= 2) break;
  }
}

void test_5() {
  std::cout << std::endl << "test_5_1()" << std::endl;
  test_5_1();

  std::cout << std::endl << "test_5_2()" << std::endl;
  test_5_2();

  std::cout << std::endl << "test_5_3()" << std::endl;
  test_5_3();

  std::cout << std::endl << "test_5_4()" << std::endl;
  test_5_4();
}
 