#include "../test.h"

#pragma once

class Logger {
  public:
  Logger() = default;
  virtual ~Logger() {};
  virtual void LogMessage(const char* fmt, const u64 log_res) {};
};