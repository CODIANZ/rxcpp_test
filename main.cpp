#include <iostream>
#include <rxcpp/rx.hpp>

int main() {
  rxcpp::observable<>::just("hello rxcpp test")
  .subscribe([=](auto txt){
    std::cout << txt << std::endl;
  });
}
