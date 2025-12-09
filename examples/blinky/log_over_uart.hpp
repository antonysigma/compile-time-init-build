#pragma once

#include <cib/cib.hpp>
#include <cstdint>
#include <log/catalog/encoder.hpp>
#include <log/log.hpp>
#include <msg/message.hpp>
#include <stdx/span.hpp>

// Must be the last one.
#include <Arduino.h>

#define BLINKY_LEGACY_BINARY_LOGGER

namespace serial_logger {

struct config {
    struct {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename FmtResult>
    void log(FilenameStringType f, LineNumberType n, FmtResult const &fr)
        {
            using MyString = decltype(logging::binary::detail::to_message<decltype(fr.str), -1>());
            const auto msg_id = catalog<MyString>();

            // Assuming number of unique log string is <= 256.
            Serial.write(static_cast<uint8_t>(msg_id & 0xff));
            fr.args.apply([](auto... args) { logArgs(args...); });
        }

        template <typename... Args>
        static void logArgs(uint8_t const t, Args &&...args) {
            Serial.write(t);
            logArgs(args...);
        }

        static void logArgs() {}
    } logger;

    [[noreturn]] static void terminate() {
        // terminate in some way
    }
};


namespace defn {
using msg::at;
using msg::dword_index_t;
using msg::field;
using msg::message;
using msg::operator""_msb;
using msg::operator""_lsb;

// Define a message type for the custom binary format.
// For simplicity, this message is just the 32-bit string ID.
using id_f = field<"id", std::uint32_t>::located<at{dword_index_t{0}, 31_msb, 0_lsb}>;
using id_msg_t = message<"id", id_f>;
}  // namespace defn

// Provide a builder: a structure with a build function that takes
// various arguments and returns an (owning) message.
struct builder : logging::mipi::default_builder<> {
    template <auto Level, logging::packable... Ts>
    static auto build(string_id, module_id, logging::mipi::unit_t, Ts...) {
        using namespace msg;
        return owning<defn::id_msg_t>{"id"_field = 42};
    }
};

struct writer {
    template <std::size_t N>
    auto operator()(stdx::span<std::uint32_t const, N> packet) const {
        static_assert(N <= 2);

        for (const auto &c : packet) {
            Serial.write(static_cast<uint8_t>(c & 0xff));
        }
    }
};

} // namespace serial_logger

#ifdef BLINKY_LEGACY_BINARY_LOGGER
template <>
inline auto logging::config<> = serial_logger::config{};
#else
template <>
inline auto logging::config<> = logging::binary::config{serial_logger::writer{}};
#endif
