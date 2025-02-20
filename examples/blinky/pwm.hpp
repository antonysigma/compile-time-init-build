#pragma once
#include "config.hpp"
#include "core.hpp"
#include "interfaces.hpp"

#include <cib/cib.hpp>
#include <log/log.hpp>

#include <Arduino.h>
#include <cmath>

template <class pin_a, class pin_b, bool phase_shift> struct pwm_output {
    constexpr static enum pwm_mode_t : uint8_t {
        PCPWM8 = 1,
        PCPWM9 = 2,
        PCPWM10 = 3,
    } phase_correct_mode{PCPWM8};
    constexpr static uint16_t resolution =
        (1UL << (phase_correct_mode + 7)) - 1;
    constexpr static auto timer_id = pin_a::timer_id;
    constexpr static auto this_timer = timer[timer_id];

    constexpr static void writeA(float duty_cycle) {
        pin_a::ocr() = static_cast<uint8_t>(round(resolution * duty_cycle));
    }

    constexpr static void writeB(float duty_cycle) {
        // Inversion of +ve duty cycle is simply the -ve duty cycle.
        pin_b::ocr() =
            static_cast<uint8_t>(round(resolution * (1 - duty_cycle)));
    }

    constexpr static auto setup_pwm = flow::action("SetupPWM"_sc, []() {
        static_assert(pin_a::timer_id == pin_b::timer_id);

        writeA(0.1f);
        writeB(0.4f);
    });

    constexpr static auto set_pin = flow::action("set_pin"_sc, []() {
        static_assert(this_timer.usage == Timer::PHASE_CORRECT_PWM);
        constexpr auto prescaler = static_cast<uint16_t>(
            round(oscillator_freq / this_timer.freq / 4 / resolution));

        // Non-inverting on OCRA, inverting on OCRB.
        constexpr auto non_inverting = 0b1011;

        // Set the pin to digital output
        pin_a::port() = pin_a::port() | pin_a::mask;
        pin_b::port() = pin_b::port() | pin_b::mask;

        if constexpr (timer_id == 1) {
            TCCR1A = (phase_correct_mode & 0b11) | (non_inverting << 4);
            TCCR1B = (phase_correct_mode >> 2 << 3) |
                     prescalerRegisterValue<prescaler>();
        } else { // Timer 2
            TCCR2A = (phase_correct_mode & 0b11) | (non_inverting << 4);
            TCCR2B = (phase_correct_mode >> 2 << 3) |
                     prescalerRegisterValue<prescaler>();
        }

        if constexpr (phase_shift) {
            if constexpr (timer_id == 1) {
                // 90-deg phase shift
                static_assert(resolution <= 255);
                TCNT1L = resolution / 2;
            } else {
                TCNT2 = resolution / 2;
            }
        }
    });

    constexpr static auto config = cib::config( //
        cib::extend<RuntimeInit>(core_init::disable_irq >> set_pin >>
                                 setup_pwm >> core_init::enable_irq) //
    );
};