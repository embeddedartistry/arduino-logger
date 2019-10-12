#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

#include <AVRCircularBufferLogger.h>

using PlatformLogger = PlatformLogger_t<AVRCircularLogBufferLogger<1024>>;

#endif
