#pragma once
#include "units.hpp"

#include <array>

using units::literals::operator""_kHz;
using units::literals::operator""_ms;

struct Timer {
    units::Microsecond<uint32_t> interrupt_interval{};
    units::KiloHertz freq{};
    enum usage_t { INTERRUPT, PHASE_CORRECT_PWM } usage{INTERRUPT};
};

constexpr auto oscillator_freq = 32e3_kHz;

constexpr std::array timer{Timer{10_ms, 16e3_kHz, Timer::INTERRUPT},
                           Timer{0_ms, 3.92_kHz, Timer::PHASE_CORRECT_PWM}};

enum digital_pin_t : uint8_t { D9 = (1 << 1), D10 = (1 << 2) };