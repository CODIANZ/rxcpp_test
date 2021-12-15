#include <iostream>
#include <rxcpp/rx.hpp>

/* void的なもの */
struct unit {};

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

/**
 * スコープの出入りをカウントする
 * 関数がどのタイミングで終了するかを確認するため
 **/
struct scope_counter {
  static std::atomic_int enter_;
  static std::atomic_int leave_;
  static void dump() {
    /* 手抜き */
    const int e = enter_;
    const int l = leave_;
    std::cout
      << "current entries: " << e - l
      <<  ", total enter: " << e
      << ", total leave: " << l
      << std::endl;
  }
  static void reset() {
    enter_ = 0;
    leave_ = 0;
  }
  int self_;
  scope_counter() : self_(enter_++) {
    std::cout << "enter: " << self_ << std::endl;
  }
  ~scope_counter() {
    std::cout << "leave: " << self_ << std::endl;
    leave_++;
  }
};

/* static */ std::atomic_int scope_counter::enter_;
/* static */ std::atomic_int scope_counter::leave_;

/** 
 * n - 1 回目 まで failure を返却
 * n 回目で success を返却
 * n + 1 回目以降は failure を返却
 **/
enum class result { success, failure };
class some_api {
private:
  std::atomic_int n_;
public:
  some_api(int n) : n_(n) {}
  ~some_api() = default;
  auto call() -> rxcpp::observable<result>
  {
    const int n = --n_;
    std::cout << "fire: " << n << std::endl;
    return rxcpp::observable<>::just(n == 0 ? result::success : result::failure, rxcpp::observe_on_new_thread());
  }
};

class some_api_sync {
private:
  std::atomic_int n_;
public:
  some_api_sync(int n) : n_(n) {}
  ~some_api_sync() = default;
  auto call() -> rxcpp::observable<result>
  {
    const int n = --n_;
    std::cout << "fire: " << n << std::endl;
    return rxcpp::observable<>::just(n == 0 ? result::success : result::failure);
  }
};

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
  ~some_state() = default;
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

/** 繰り返しで retry() を使う */
void test_1_1() {
  auto api = std::make_shared<some_api>(5);

  auto sbsc = rxcpp::observable<>::just(unit{})
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
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

/** 再帰が深すぎてクラッシュする */
void test_2_1() {
  auto api = std::make_shared<some_api_sync>(1000);

  auto sbsc = rxcpp::observable<>::just(unit{})
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
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

/** 再帰を抑止する */
void test_2_2() {
  auto api = std::make_shared<some_api_sync>(1000);

  auto sbsc = rxcpp::observable<>::just(unit{})
  .observe_on(observe_on_async())
  .flat_map([=](unit){
    return api->call();
  }).as_dynamic()
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

/** 引き金を引いてから仕掛けるパターン（取りこぼしする） */
void test_3_1() {
  auto fn = [](std::shared_ptr<some_state> state){
    state->start();
    return state->observable();
  };

  auto state = std::make_shared<some_state>();

  auto sbsc = fn(state)
  .take(5)
  .subscribe([=](int n){
    std::cout << n << std::endl;
  }, [](std::exception_ptr){
  }, [=](){
  });
  state->end();

  while(sbsc.is_subscribed()) {}
}

/** 仕掛けてから引き金を引いくパターン（取りこぼさない） */
void test_3_2() {
  auto fn = [](std::shared_ptr<some_state> state){
    return ready_set_go([state](){
      state->start();
    }, state->observable());
  };

  auto state = std::make_shared<some_state>();

  auto sbsc = fn(state)
  .take(5)
  .subscribe([=](int n){
    std::cout << n << std::endl;
  }, [](std::exception_ptr){
  }, [=](){
  });
  state->end();

  while(sbsc.is_subscribed()) {}
}

int main() {
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

  /** test_2_1() は実行するとクラッシュします */
  // std::cout << std::endl << "test_2_1()" << std::endl;
  // test_2_1();
  // scope_counter::reset();

  std::cout << std::endl << "test_2_2()" << std::endl;
  test_2_2();
  scope_counter::reset();

  std::cout << std::endl << "test_3_1()" << std::endl;
  test_3_1();
  scope_counter::reset();

  std::cout << std::endl << "test_3_2()" << std::endl;
  test_3_2();
  scope_counter::reset();
}
