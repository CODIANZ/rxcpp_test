#if !defined(__h_scope_counter__)
#define __h_scope_counter__

#include <iostream>
#include <atomic>

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

#endif /* !defined(__h_scope_counter__) */
