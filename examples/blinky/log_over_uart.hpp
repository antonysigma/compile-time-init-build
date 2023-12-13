#pragma once
#include <cib/cib.hpp>
#include <log/catalog/catalog.hpp>

#include <Arduino.h>

namespace serial_logger {
struct config {
    struct {
    template <typename Env, typename FilenameStringType,
              typename LineNumberType, typename FmtResult>
    void log(FilenameStringType f, LineNumberType n, FmtResult const &fr)
        {
            //using Message = message<Level, MsgType>;
            //uint32_t const msg_id = catalog<Message>();
            const uint32_t msg_id = 0;

            // Assume number of messages < 256.
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
} // namespace serial_logger

template <> inline auto logging::config<> = serial_logger::config{};
