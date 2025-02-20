
#include <cmath>

// <cmath> should be included before <Arduino.h> to avoid the C-macros abs()
#include "blink.hpp"
#include "core.hpp"
#include "log_over_uart.hpp"
#include "pwm.hpp"
#include "serial.hpp"

struct registered_interfaces {
    constexpr static auto config =
        cib::config(cib::exports<RuntimeInit>,      //
                    cib::exports<OnTimerInterrupt>, //
                    cib::exports<MainLoop>);
};

struct project {
    constexpr static auto config =
        cib::components<registered_interfaces,                            //
                        core_init,                                        //
                        serial_init,                                      //
                        blink<13, 1'000_ms>,                              //
                        pwm_output<digital_pin_t::D9, digital_pin_t::D10> //
                        >;
};

cib::nexus<project> nexus{};

ISR(TIMER0_COMPA_vect) { nexus.service<OnTimerInterrupt>(); }

int main() {
    nexus.service<RuntimeInit>();

    for (;;) {
        nexus.service<MainLoop>();
    }

    return 0;
}