#if !defined(__h_ready_set_go__)
#define __h_ready_set_go__

#include "common.h"

/** o を subscribe した後、 f() を呼び出す。 o が発行した値を観測可能な observable を返却する。 */
template <typename T> rxcpp::observable<T> ready_set_go(std::function<void()> f, rxcpp::observable<T> o) {
  return rxcpp::observable<>::create<T>([f, o](rxcpp::subscriber<T> s){
    o.subscribe([s](const T& v){
      s.on_next(v);
    }, [s](std::exception_ptr err){
      s.on_error(err);
    }, [s](){
      s.on_completed();
    });
    try{
      f();
    }
    catch(std::exception& err){
      s.on_error(std::make_exception_ptr(err));
    }
  });
}

#endif /* !defined(__h_ready_set_go__) */
