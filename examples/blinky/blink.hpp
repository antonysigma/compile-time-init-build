#pragma once

#include "config.hpp"
#include "core.hpp"
#include "interfaces.hpp"

#include <cib/cib.hpp>
#include <log/log.hpp>

#include <Arduino.h>

template <uint8_t led_pin, units::Microsecond<uint32_t> blink_interval>
struct blink {
    static inline bool volatile is_interrupted = false;

    constexpr static auto set_pin =
        flow::action("set_pin"_sc, []() { ::pinMode(led_pin, OUTPUT); });

    constexpr static auto config = cib::config(                        //
        cib::extend<RuntimeInit>(core_init::disable_usart >> set_pin), //
        cib::extend<OnTimerInterrupt>([]() {
            constexpr auto duration_steps = static_cast<uint32_t>(
                blink_interval / timer[0].interrupt_interval);
            static uint8_t step = 0;
            static_assert(std::log2(duration_steps) <= sizeof(step) * 8,
                          "Need more than bits to implement delay()");
            static_assert(std::log2(duration_steps) >
                              std::max(0, int(sizeof(step)) - 1) * 8,
                          "Wasteful compute");
            if (++step < duration_steps) {
                return;
            }
            step = 0;
            is_interrupted = true;
        }), //
        cib::extend<MainLoop>([]() {
            static uint8_t state = HIGH;

            if (!is_interrupted) {
                return;
            }

            is_interrupted = false;
            digitalWrite(led_pin, state);
            CIB_INFO("LED {:02d} = {:02d}!", led_pin, state);
            state = !state;
        }) //
    );
};