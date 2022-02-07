#if !defined(__h_debug_scheduler__)
#define __h_debug_scheduler__

#include "common.h"

#if !defined(USE_ANOTHER_RXCPP)

/** デバッグ用のスケジューラ */
class debug_scheduler : public rxcpp::schedulers::scheduler_interface {
private:
  class debug_worker : public rxcpp::schedulers::worker_interface {
  private:
    rxcpp::composite_subscription m_lifetime;
    const std::string m_name;
  public:
    explicit debug_worker(rxcpp::composite_subscription cs, const std::string& name) : m_lifetime(cs), m_name(name) {}
    virtual ~debug_worker() { m_lifetime.unsubscribe(); }
    virtual clock_type::time_point now() const override { return clock_type::now(); }
    virtual void schedule(const rxcpp::schedulers::schedulable& scbl) const override {
      schedule(now(), scbl);
    }
    virtual void schedule(rxcpp::schedulers::scheduler_interface::clock_type::time_point when, const rxcpp::schedulers::schedulable& scbl) const override {
      if (scbl.is_subscribed()) {
        std::cout << "[[schedule]] " << m_name << " {" << std::endl;
        scbl(rxcpp::schedulers::recursion(true).get_recurse());
        std::cout << "[[schedule]] " << m_name << " }" << std::endl;
      }
    }
  };
  const std::string m_name;

public:
  debug_scheduler(const std::string& name) : m_name(name) {}
  virtual ~debug_scheduler() = default;
  virtual rxcpp::schedulers::scheduler_interface::clock_type::time_point now() const override {
    return rxcpp::schedulers::scheduler_interface::clock_type::now();
  }
  virtual rxcpp::schedulers::worker create_worker(rxcpp::composite_subscription cs) const override {
    return rxcpp::schedulers::worker(cs, std::make_shared<debug_worker>(cs, m_name));
  }
};

/** observe_on() でこいつを使う */
rxcpp::observe_on_one_worker observe_on_debug(const std::string& name) {
  return rxcpp::observe_on_one_worker(rxcpp::schedulers::make_scheduler<debug_scheduler>(name));
}

#else

class debug_scheduler_interface : public another_rxcpp::scheduler_interface {
private:
  const std::string m_name;

public:
  debug_scheduler_interface(const std::string& name) noexcept :
    scheduler_interface(schedule_type::direct),
    m_name(name) {}
  virtual ~debug_scheduler_interface() = default;

  virtual void run(call_in_context_fn_t call_in_context) noexcept override {
    call_in_context();
  }

  virtual void detach() noexcept override {
  }

  virtual void schedule(const function_type& f) noexcept override {
    std::cout << "[[schedule]] " << m_name << " {" << std::endl;
    f();
    std::cout << "[[schedule]] " << m_name << " }" << std::endl;
  }
};

inline auto observe_on_debug(const std::string& name) noexcept {
  return [name]{
    return another_rxcpp::scheduler(
      std::make_shared<debug_scheduler_interface>(name)
    );
  };
}


#endif

#endif /* !defined(__h_debug_scheduler__) */