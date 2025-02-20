#pragma once

#include "config.hpp"
#include "interfaces.hpp"

#include <cib/cib.hpp>

#include <wiring_private.h>

template <uint16_t factor> constexpr uint8_t prescalerRegisterValue() {
    static_assert(factor == 8 || factor == 64 || factor == 256 ||
                  factor == 1024);
    switch (factor) {
    case 1024:
        return (1 << CS02) | (1 << CS00);
    case 256:
        return (1 << CS02);
    case 64:
        return (1 << CS01) | (1 << CS00);
    case 8:
        return (1 << CS01);
    default:
        return 0;
    }
}

template <uint16_t prescaler_value, units::Microsecond<uint32_t> time_interval>
constexpr uint8_t overflowRegisterValue() {
    constexpr auto overflow_value = static_cast<uint32_t>(
        timer[0].freq * time_interval / prescaler_value - 1);
    static_assert(
        overflow_value < 255,
        "Overflow register value > 255; insufficient clock prescaler value?");
    return static_cast<uint8_t>(overflow_value);
}
static_assert(overflowRegisterValue<64, 1_ms>() == 0xF9);

template <units::KiloHertz timer_freq> constexpr uint8_t clockPrescaler() {
    constexpr auto division =
        static_cast<uint32_t>(oscillator_freq / timer_freq);

    static_assert(division <= 256);
    static_assert(__builtin_popcount(division) == 1, "Must be power of two");
    return std::log2(division);
}
static_assert(clockPrescaler<16e3_kHz>() == 0x01);
static_assert(clockPrescaler<8e3_kHz>() == 0x02);
static_assert(clockPrescaler<4e3_kHz>() == 0x03);

struct core_init {
    constexpr static auto clk_init = flow::action("ClkInit"_sc, []() {
        cli();
        // Enable clock prescaler pin change.
        CLKPR = 0x80;

        // With 4 clock cycles, update the clock division factor.
        CLKPR = clockPrescaler<timer[0].freq>();
        sei();
    });
    constexpr static auto timer0_init = flow::action("TimerInit"_sc, []() {
        constexpr auto prescaler = 1024UL;

        cli();
        TCCR0A = TCCR0A | (1 << WGM01); // Set the CTC mode
        OCR0A = overflowRegisterValue<
            prescaler, timer[0].interrupt_interval>(); // Set the value for 1ms
        TIMSK0 = TIMSK0 | (1 << OCIE0A); // Set the interrupt request
        TCCR0B =
            TCCR0B |
            prescalerRegisterValue<prescaler>(); // Set the prescale 1/64 clock
        sei();                                   // Enable interrupt
    });

    constexpr static auto disable_usart = flow::action("DisableUSART"_sc, []() {
#if defined(UCSRB)
        UCSRB = 0;
#elif defined(UCSR0B)
        UCSR0B = 0;
#endif
    });

    constexpr static auto config = cib::config(
        cib::extend<RuntimeInit>(clk_init >> timer0_init >> disable_usart));
};