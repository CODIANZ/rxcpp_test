#if !defined(__h_something__)
#define __h_something__

#include "common.h"

template<typename T = void> class something;

template <> class something<void> {
private:
  class retry_trigger : public std::exception {};
public:
  [[noreturn]] static void retry() {
    throw retry_trigger{};
  }
  template<typename T, typename TT = typename std::remove_reference<T>::type>
  static something<TT> success(T v) {
    return something<TT>(
      rxcpp::observable<>::just(std::forward<T>(v))
    );
  }
  template<typename T> static something<T> error(std::exception_ptr err) {
    return something<T>(
      rxcpp::observable<>::error<T>(err)
    );
  }
};

template<typename T> class something {
friend class something<>;
private:
  rxcpp::observable<T> o_;
  something(rxcpp::observable<T> o) : o_(o) {}
public:
  rxcpp::observable<T> proceed() { return o_; }
};

#endif /* !defined(__h_something__) */