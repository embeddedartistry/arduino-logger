#ifndef TEST_HELPER_HPP_
#define TEST_HELPER_HPP_

#include <ArduinoLogger.h>
#include <string>

constexpr size_t prefix_len = 4;
constexpr size_t int_prefix_len = 6;

static inline std::string construct_log_string(log_level_e level, const char* str)
{
	return std::string(LOG_LEVEL_TO_SHORT_C_STRING(level)) + std::string(str);
}

extern std::string log_buffer_output;

#endif
