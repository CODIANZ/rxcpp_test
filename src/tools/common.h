#if !defined(__h_rx__)
#define __h_rx__

#if 0
  #include <rxcpp/rx.hpp>
#else
  #define USE_ANOTHER_RXCPP
  #define SUPPORTS_RXCPP_COMPATIBLE
  #define SUPPORTS_OPERATORS_IN_OBSERVABLE
  #include <another-rxcpp/rx.h>
#endif

#endif /* !defined(__h_rx__) */
