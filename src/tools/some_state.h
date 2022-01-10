#if !defined(__h_some_state__)
#define __h_some_state__

#include "common.h"

/** 
 * startで hot observable が値発行を開始する
 * 発行する値は 0〜　順に +1 される
 **/
struct some_state {
private:
  rxcpp::subjects::subject<int> sbj_;
  rxcpp::subscription sbs_;
public:
  some_state() = default;
  ~some_state() {
    end();
  }
  auto observable() { return sbj_.get_observable(); }
  void start(){
    sbj_.get_subscriber().on_next(0);
    sbs_ = rxcpp::observable<>::interval(std::chrono::milliseconds(200), rxcpp::observe_on_new_thread())
    .tap([=](int n){
      sbj_.get_subscriber().on_next(n);
    })
    .subscribe();
  }
  void end() {
    sbj_.get_subscriber().on_completed();
    sbs_.unsubscribe();
  }
};

#endif /* !defined(__h_some_state__) */