#include "scope_counter.h"

/* static */ std::atomic_int scope_counter::enter_;
/* static */ std::atomic_int scope_counter::leave_;
