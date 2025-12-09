#pragma once
#include <Arduino.h>

#include <cmath>

#include "config.hpp"
#include "core.hpp"
#include "interfaces.hpp"
#include "log_over_uart.hpp"

namespace components {

#ifndef BLINKY_LEGACY_BINARY_LOGGER
CIB_LOG_ENV(logging::binary::get_builder, serial_logger::builder{});
#endif

template <uint8_t led_pin> struct blink {
    static inline bool volatile is_interrupted = false;

    constexpr static auto set_pin =
        flow::action<"set_pin">([]() { ::pinMode(led_pin, OUTPUT); });

    constexpr static auto config = cib::config(                          //
        cib::extend<RuntimeInit>(core_init::disable_usart >> *set_pin),  //
        cib::extend<OnTimerInterrupt>([]() {
            constexpr auto duration_steps =
                blink_interval_ms / timer_interrupt_internal_ms;
            static uint8_t step = 0;
            static_assert(std::log2(duration_steps) <= sizeof(step) * 8,
                          "Integer 'bit' needs more bits to implement delay()");
            static_assert(std::log2(duration_steps) >
                              std::max(0, int(sizeof(step)) - 1) * 8,
                          "Wasteful compute");
            if (++step < duration_steps) {
                return;
            }
            step = 0;
            is_interrupted = true;
        }),  //
        cib::extend<MainLoop>([]() {
            static uint8_t state = HIGH;

            if (!is_interrupted) {
                return;
            }

            is_interrupted = false;
            CIB_INFO("Writing to GPIO...");
            digitalWrite(led_pin, state);
            CIB_INFO("LED {} = {}!", led_pin, state);
            state = !state;
        })  //
    );
};
}
