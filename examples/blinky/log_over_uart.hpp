#pragma once
#include <cib/cib.hpp>
#include <log/catalog/catalog.hpp>

#include <Arduino.h>

namespace serial_logger {
struct config {
    struct {
        template <logging::level Level, typename FilenameStringType,
                  typename LineNumberType, typename MsgType>
        auto log(FilenameStringType, LineNumberType,
                 MsgType const &msg) -> void {
            using Message = message<Level, MsgType>;
            uint32_t const msg_id = catalog<Message>();

            // Assume number of messages < 256.
            Serial.write(static_cast<uint8_t>(msg_id & 0xff));

            msg.args.apply([](auto... args) { logArgs(args...); });
        }

        template <typename... Args>
        static void logArgs(uint8_t const t, Args &&...args) {
            Serial.write(t);
            logArgs(args...);
        }

        static void logArgs() {}
    } logger;

    [[noreturn]] static auto terminate() -> void {
        // terminate in some way
    }
};
} // namespace serial_logger

template <> inline auto logging::config<> = serial_logger::config{};