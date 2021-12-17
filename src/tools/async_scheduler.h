#if !defined(__h_async_scheduler__)
#define __h_async_scheduler__

#include <rxcpp/rx.hpp>

/** 簡単なスケジューラ例 （std::async を使ってみる） */
class async_scheduler : public rxcpp::schedulers::scheduler_interface {
private:
  class async_worker : public rxcpp::schedulers::worker_interface {
  private:
    rxcpp::composite_subscription m_lifetime;
  public:
    explicit async_worker(rxcpp::composite_subscription cs) : m_lifetime(cs) {}
    virtual ~async_worker() { m_lifetime.unsubscribe(); }
    virtual clock_type::time_point now() const override { return clock_type::now(); }
    virtual void schedule(const rxcpp::schedulers::schedulable& scbl) const override {
      schedule(now(), scbl);
    }
    virtual void schedule(rxcpp::schedulers::scheduler_interface::clock_type::time_point when, const rxcpp::schedulers::schedulable& scbl) const override {
      if (scbl.is_subscribed()) {
        auto THIS = shared_from_this();
        std::async(std::launch::async, [THIS, scbl, when](){
          std::this_thread::sleep_until(when);
          scbl(rxcpp::schedulers::recursion(true).get_recurse());
        });
      }
    }
  };
public:
  async_scheduler() = default;
  virtual ~async_scheduler() = default;
  virtual rxcpp::schedulers::scheduler_interface::clock_type::time_point now() const override {
    return rxcpp::schedulers::scheduler_interface::clock_type::now();
  }
  virtual rxcpp::schedulers::worker create_worker(rxcpp::composite_subscription cs) const override {
    return rxcpp::schedulers::worker(cs, std::make_shared<async_worker>(cs));
  }
};

/** observe_on() でこいつを使う */
rxcpp::observe_on_one_worker observe_on_async() {
  return rxcpp::observe_on_one_worker(rxcpp::schedulers::make_scheduler<async_scheduler>());
}

#endif /* !defined(__h_async_scheduler__) */