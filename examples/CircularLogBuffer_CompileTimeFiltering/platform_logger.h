#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

// This will change the compile-time log level to compile-out logdebug messages
#define LOG_LEVEL LOG_LEVEL_INFO
#include <CircularBufferLogger.h>

using PlatformLogger = PlatformLogger_t<CircularLogBufferLogger<1024>>;

#endif
