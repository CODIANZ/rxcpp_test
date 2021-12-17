#include "test_7.h"

#include <iostream>
#include "../tools/some_api.h"
#include "../tools/something.h"

void test_7_1() {
  some_api api(5);

  auto sbsc = api.call()
  .map([=](result r){
    if(r == result::success) {
      std::cout << "success" << std::endl;
      return something<>::success(r); /* 正常 */
    }
    std::cout << "failure -> retry" << std::endl;
    something<>::retry(); /* retryオペレータを反応させます */
  })
  .on_error_resume_next([](std::exception_ptr err){
    /**
     * このサンプルコードでは発生することはないのですが、
     * 例えば、再試行させたくないエラーを、ここでsomethingとして値を発行します。
     * そして、retryの後段で flat_map して下流にエラーを発行します。
     */
    return rxcpp::observable<>::just(something<>::error<result>(err));    
  })
  .retry()
  .flat_map([](something<result> o){
    /** ここで just<result> から error<result> に接続されます */
    return o.proceed();
  })
  .subscribe([](result x){
    std::cout << "on next" << std::endl;
  }, [](std::exception_ptr err){
    std::cout << "on error" << std::endl;
  }, [](){
    std::cout << "on complete" << std::endl;
  });

  while(sbsc.is_subscribed()) {}
}

void test_7_2() {
  some_api api(5);
  auto error_count = std::make_shared<int>(0);

  auto sbsc = api.call()
  .map([=](result r){
    if(r == result::failure) {
      *error_count = *error_count + 1;
      std::cout << "error count " << *error_count << std::endl;
      if(*error_count == 2){
        std::cout << "pass the error" << std::endl;
        return something<>::error<result>(std::make_exception_ptr(std::exception()));
      }
    }
    std::cout << "failure -> retry" << std::endl;
    something<>::retry();
  })
  .retry()
  .flat_map([](something<result> o){
    /** ここで just<result> から error<result> に接続されます */
    return o.proceed();
  })
  .subscribe([](result x){
    std::cout << "on next" << std::endl;
  }, [](std::exception_ptr err){
    std::cout << "on error" << std::endl;
  }, [](){
    std::cout << "on complete" << std::endl;
  });

  while(sbsc.is_subscribed()) {}
}

void test_7() {
  std::cout << std::endl << "test_7_1()" << std::endl;
  test_7_1();

  std::cout << std::endl << "test_7_2()" << std::endl;
  test_7_2();
}
