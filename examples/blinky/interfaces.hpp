#pragma once

#include <cib/cib.hpp>
class RuntimeInit : public flow::service<> {};
class OnTimerInterrupt : public cib::callback_meta<> {};
class MainLoop : public cib::callback_meta<> {};
