// the "main" application is a stub that calls into the library code

#if defined(ARDUINO)
#include <Arduino.h>
#endif

namespace lib {
auto lib_func() -> void;
}

auto main() -> int {
#if defined(ARDUINO)
  Serial.begin(115200);
#endif
  lib::lib_func();
  return 0;
}
