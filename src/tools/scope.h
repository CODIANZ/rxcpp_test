#if !defined(__h_scope__)
#define __h_scope__

#include <iostream>
#include <sstream>

/** 関数の入退出を確認 */
struct scope {
  const std::string name_;
  scope(const std::string& name) : name_(name) {
    std::cout << "enter:" << name_ << std::endl;
  }
  scope(const std::string& name, int n) :
  name_((std::stringstream() << name << " (" << n << ")").str()) {
    std::cout << "enter:" << name_ << std::endl;
  }
  ~scope() {
    std::cout << "leave:" << name_ << std::endl;
  }
};

#endif /* !defined(__h_scope__) */