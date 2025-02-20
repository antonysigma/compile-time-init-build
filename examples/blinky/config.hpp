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
                           Timer{0_ms, 3.92_kHz, Timer::PHASE_CORRECT_PWM},
                           Timer{0_ms, 3.92_kHz, Timer::PHASE_CORRECT_PWM}};

template <uint8_t id> struct DigitalPin {
    constexpr static uint8_t port_addr{[]() {
        static_assert((id >= 9 && id <= 11) || (id == 3));
        switch (id) {
        case 9:
        case 10:
        case 11:
            return 0x24;
        case 3:
            return 0x2a;
        }
    }()};

    static uint8_t volatile &port() {
        return *reinterpret_cast<uint8_t volatile *>(port_addr);
    }

    constexpr static uint8_t mask{[]() {
        switch (id) {
        case 9:
        case 10:
        case 11:
            return 1UL << (id - 8);
        case 3:
            return 1UL << 3;
        }
    }()};

    constexpr static uint8_t timer_id{(id == 9 || id == 10) ? 1 : 2};

    constexpr static uint8_t ocr_addr{[]() {
        switch (id) {
        case 9:
            return 0x88;
        case 10:
            return 0x8a;
        case 11:
            return 0xb3;
        case 3:
            return 0xb4;
        }
    }()};

    static uint8_t volatile &ocr() {
        return *reinterpret_cast<uint8_t volatile *>(ocr_addr);
    }

    constexpr DigitalPin() { static_assert(mask != 0); }
};