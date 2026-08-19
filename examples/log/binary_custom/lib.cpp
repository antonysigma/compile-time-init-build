#if defined(ARDUINO)
#include <Arduino.h>
#endif

#include <conc/concurrency.hpp>

#include "logger.hpp"

namespace {
struct avr_conc_policy {
  template <typename = void, stdx::invocable F, stdx::predicate... Pred>
    requires(sizeof...(Pred) < 2)
  static inline auto call_in_critical_section(F &&f, Pred &&...)
      -> decltype(std::forward<F>(f)()) {
    return std::forward<F>(f)();
  }
};
} // namespace

#if defined(SIMULATE_FREESTANDING)
template <> inline auto conc::injected_policy<> = avr_conc_policy{};
#endif

namespace lib {
auto lib_func() -> void { CIB_INFO("Hello"); }
} // namespace lib
