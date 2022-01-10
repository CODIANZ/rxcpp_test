#if !defined(__h_inflow_restriction__)
#define __h_inflow_restriction__

#include "common.h"
#include "../tools/sem.h"

template <int N>
class inflow_restriction {
private:
  sem<N> m_sem;

public:
  inflow_restriction() = default;
  inflow_restriction(const inflow_restriction&) = delete;
  ~inflow_restriction() = default;

  template <typename T>
  rxcpp::observable<T> enter(rxcpp::observable<T> o){
    return rxcpp::observable<>::create<T>([=](rxcpp::subscriber<T> s){
      m_sem.lock();
      o.subscribe([=](T v){
        s.on_next(v);
      }, [=](std::exception_ptr e){
        s.on_error(e);
        m_sem.unlock();
      }, [=](){
        s.on_completed();
        m_sem.unlock();
      });
    });
  }
};

#endif /* !defined(__h_inflow_restriction__) */