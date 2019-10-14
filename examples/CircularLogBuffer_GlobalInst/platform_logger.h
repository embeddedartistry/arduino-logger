#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

#include <CircularBufferLogger.h>

using PlatformLogger = PlatformLogger_t<CircularLogBufferLogger<1024>>;

#endif
