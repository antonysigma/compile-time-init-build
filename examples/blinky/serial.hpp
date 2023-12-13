#pragma once

#include "interfaces.hpp"

#include <cib/cib.hpp>

#include <Arduino.h>

struct serial_init {
    constexpr static auto enable_uart = flow::action("SerialInit"_sc, []() {
        Serial.begin(115'200);
        while (!Serial)
            ;
    });

    constexpr static auto config = cib::config(cib::extend<RuntimeInit>(
        core_init::disable_usart >> serial_init::enable_uart));
};