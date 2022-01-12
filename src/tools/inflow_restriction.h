#if !defined(__h_inflow_restriction__)
#define __h_inflow_restriction__

#include "common.h"
#include "../tools/sem.h"

template <int N>
class inflow_restriction {
private:
  using sem_type = sem<N>;
  std::shared_ptr<sem_type> sem_;

public:
  inflow_restriction() :
    sem_(std::make_shared<sem_type>()) {}
  inflow_restriction(const inflow_restriction&) = delete;
  ~inflow_restriction() = default;

  template <typename T>
  rxcpp::observable<T> enter(rxcpp::observable<T> o){
    return rxcpp::observable<>::create<T>([=](rxcpp::subscriber<T> s){
      auto sem = sem_;
      sem->lock();
      o.subscribe([s](T v){
        s.on_next(v);
      }, [s, sem](std::exception_ptr e){
        s.on_error(e);
        sem->unlock();
      }, [s, sem](){
        s.on_completed();
        sem->unlock();
      });
    });
  }
};

#endif /* !defined(__h_inflow_restriction__) */