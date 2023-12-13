#pragma once

#include <cib/cib.hpp>
using cib::RuntimeInit;
class OnTimerInterrupt : public callback::service<> {};
class MainLoop : public callback::service<> {};
