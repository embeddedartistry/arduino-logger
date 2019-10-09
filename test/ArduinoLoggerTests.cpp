#include <catch.hpp>
#include "CircularBufferLogger.h"
#include <string>

std::string log_buffer_output;

void _putchar(char character)
{
	log_buffer_output += character;
}

TEST_CASE("Create a logger", "[ArduinoLoggerg]")
{
	CircularLogBufferLogger<1024> l;
	CHECK(0 == l.size());
	CHECK(1024 == l.capacity());
	CHECK(true == l.enabled());
	CHECK(false == l.echo());
	CHECK(LOG_LEVEL_LIMIT() == l.level());

	auto level = l.level(static_cast<log_level_e>(static_cast<int>(l.level()) + 1));
	CHECK(LOG_LEVEL_LIMIT() == level);
}

TEST_CASE("Create a logger with a system timer", "[ArduinoLoggerg]")
{
	CircularLogBufferLogger<1024> l_notimer;

	l_notimer.log(debug, "Hello world\n");

	CHECK(l_notimer.size() == 16);

	l_notimer.flush();

	CHECK(log_buffer_output == std::string_view("<D> Hello world\n"));
}

TEST_CASE("Log interface", "[ArduinoLoggerg]")
{
	CircularLogBufferLogger<1024> l;

	// We can log slightly more data than the string length
	auto expected_size = strlen("Test str") + 1;
	l.log(log_level_e::warning, "Test str");
	CHECK(expected_size <= l.size());

	l.level(log_level_e::warning);

	// Logging at an unsupported level should result in no size change
	expected_size = l.size();
	l.log(log_level_e::debug, "Gotcha");
	CHECK(expected_size == l.size());
}

TEST_CASE("Log Level to String", "[ArduinoLoggerg]")
{
	auto crit = LOG_LEVEL_TO_C_STRING(log_level_e::critical);
	CHECK(std::string_view("critical") == std::string_view(crit));

	auto err = LOG_LEVEL_TO_C_STRING(log_level_e::error);
	CHECK(std::string_view("error") == std::string_view(err));

	auto warn = LOG_LEVEL_TO_C_STRING(log_level_e::warning);
	CHECK(std::string_view("warning") == std::string_view(warn));

	auto info = LOG_LEVEL_TO_C_STRING(log_level_e::info);
	CHECK(std::string_view("info") == std::string_view(info));

	auto debug = LOG_LEVEL_TO_C_STRING(log_level_e::debug);
	CHECK(std::string_view("debug") == std::string_view(debug));

	auto off = LOG_LEVEL_TO_C_STRING(log_level_e::off);
	CHECK(std::string_view("off") == std::string_view(off));

	// Check c_str function
	auto crit_cstr = LOG_LEVEL_TO_C_STRING(log_level_e::critical);
	CHECK(0 == strcmp(crit_cstr, "critical"));
}

TEST_CASE("Log Level to Short String", "[ArduinoLoggerg]")
{
	auto crit = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::critical);
	CHECK(0 == strcmp(LOG_LEVEL_CRITICAL_PREFIX, crit));

	auto err = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::error);
	CHECK(0 == strcmp(LOG_LEVEL_ERROR_PREFIX, err));

	auto warn = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::warning);
	CHECK(0 == strcmp(LOG_LEVEL_WARNING_PREFIX, warn));

	auto info = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::info);
	CHECK(0 == strcmp(LOG_LEVEL_INFO_PREFIX, info));

	auto debug = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::debug);
	CHECK(0 == strcmp(LOG_LEVEL_DEBUG_PREFIX, debug));

	auto off = LOG_LEVEL_TO_SHORT_C_STRING(log_level_e::off);
	CHECK(0 == strcmp("O", off));
}
