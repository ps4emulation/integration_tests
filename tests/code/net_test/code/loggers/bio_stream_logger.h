#include "../test.h"
#include "logger.h"

#include <mutex>
#include <vector>

#pragma once

class BIOStreamLogger: public Logger {
  public:
  BIOStreamLogger();
  ~BIOStreamLogger() override;

  void LogMessage(const char* fmt, const u64 log_res) override;

  private:
  static void* LoggingServerThread(void* user_arg);
  static void* LoggingClientThread(void* user_arg);
};