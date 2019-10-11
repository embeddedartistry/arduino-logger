#include <catch.hpp>
#include <CircularBufferLogger.h>
#include <string>

constexpr size_t prefix_len = 4;
constexpr size_t int_prefix_len = 6;

std::string log_buffer_output;

void _putchar(char character)
{
	log_buffer_output += character;
}

static inline std::string construct_string(log_level_e level, const char* str)
{
	return std::string(LOG_LEVEL_TO_SHORT_C_STRING(level)) + std::string(str);
}

TEST_CASE("Create a logger", "[CircularBufferLogger]")
{
	CircularLogBufferLogger<1024> logger;

	// Check default values
	CHECK(0 == logger.size());
	CHECK(1024 == logger.capacity());
	CHECK(true == logger.enabled());
	CHECK(false == logger.echo());
	CHECK(LOG_LEVEL_LIMIT() == logger.level());

	auto level = logger.level(static_cast<log_level_e>(static_cast<int>(logger.level()) + 1));
	CHECK(LOG_LEVEL_LIMIT() == level);
}

TEST_CASE("Clear test", "[CircularBufferLogger]")
{
	CircularLogBufferLogger<1024> logger;
	log_buffer_output.clear();

	logger.debug("Hello world\n");
	auto expected_size = strlen("Hello world\n") + prefix_len;
	CHECK(logger.size() == expected_size);

	logger.flush();

	CHECK(log_buffer_output == std::string_view("<D> Hello world\n"));
	CHECK(0 == logger.size());
}

TEST_CASE("Flush test", "[CircularBufferLogger]")
{
	CircularLogBufferLogger<1024> logger;
	log_buffer_output.clear();

	logger.debug("Hello world\n");
	auto expected_size = strlen("Hello world\n") + prefix_len;
	CHECK(logger.size() == expected_size);

	logger.clear();

	CHECK(log_buffer_output.size() == 0);
	CHECK(0 == logger.size());
}

TEST_CASE("Run-time Filtering", "[CircularBufferLogger]")
{
	CircularLogBufferLogger<1024> logger;
	// Change level so debug statements should be ignored
	logger.level(log_level_e::warning);

	// Logging at an unsupported level should result in no size change
	auto expected_size = logger.size();
	logger.debug("This should not be added");
	CHECK(expected_size == logger.size());
}

TEST_CASE("Log level checks", "[CircularBufferLogger]")
{
	CircularLogBufferLogger<1024> logger;
	const char* test_string = "Test str";
	auto expected_size = strlen(test_string) + prefix_len;

	log_buffer_output.clear();
	logger.critical(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_string(log_level_e::critical, test_string));

	log_buffer_output.clear();
	logger.error(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_string(log_level_e::error, test_string));

	log_buffer_output.clear();
	logger.warning(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_string(log_level_e::warning, test_string));

	log_buffer_output.clear();
	logger.info(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_string(log_level_e::info, test_string));

	log_buffer_output.clear();
	logger.debug(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_string(log_level_e::debug, test_string));
}

TEST_CASE("Log Level to String", "[CircularBufferLogger]")
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

TEST_CASE("Log Level to Short String", "[CircularBufferLogger]")
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
