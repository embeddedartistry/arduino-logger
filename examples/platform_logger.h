#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

#include <circular_buffer_logger.hpp>

using PlatformLogger =
	PlatformLogger_t<CircularLogBufferLogger<1024>>;

#endif
