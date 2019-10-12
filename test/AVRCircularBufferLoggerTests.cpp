#include "test_helper.hpp"
#include <AVRCircularBufferLogger.h>
#include <catch.hpp>
#include <string>

TEST_CASE("AVR CB: Create a logger", "[AVRCircularBufferLogger]")
{
	AVRCircularLogBufferLogger<1024> logger;

	// Check default values
	CHECK(0 == logger.size());
	CHECK(1024 == logger.capacity());
	CHECK(true == logger.enabled());
	CHECK(false == logger.echo());
	CHECK(LOG_LEVEL_LIMIT() == logger.level());

	auto level = logger.level(static_cast<log_level_e>(static_cast<int>(logger.level()) + 1));
	CHECK(LOG_LEVEL_LIMIT() == level);
}

TEST_CASE("AVR CB: Clear test", "[AVRCircularBufferLogger]")
{
	AVRCircularLogBufferLogger<1024> logger;
	log_buffer_output.clear();

	logger.debug("Hello world\n");
	auto expected_size = strlen("Hello world\n") + prefix_len;
	CHECK(logger.size() == expected_size);

	logger.flush();

	CHECK(log_buffer_output == std::string_view("<D> Hello world\n"));
	CHECK(0 == logger.size());
}

TEST_CASE("AVR CB: Flush test", "[AVRCircularBufferLogger]")
{
	AVRCircularLogBufferLogger<1024> logger;
	log_buffer_output.clear();

	logger.debug("Hello world\n");
	auto expected_size = strlen("Hello world\n") + prefix_len;
	CHECK(logger.size() == expected_size);

	logger.clear();

	CHECK(log_buffer_output.size() == 0);
	CHECK(0 == logger.size());
}

TEST_CASE("AVR CB: Run-time Filtering", "[AVRCircularBufferLogger]")
{
	AVRCircularLogBufferLogger<1024> logger;
	// Change level so debug statements should be ignored
	logger.level(log_level_e::warning);

	// Logging at an unsupported level should result in no size change
	auto expected_size = logger.size();
	logger.debug("This should not be added");
	CHECK(expected_size == logger.size());
}

TEST_CASE("AVR CB: Log level checks", "[AVRCircularBufferLogger]")
{
	AVRCircularLogBufferLogger<1024> logger;
	const char* test_string = "Test str";
	auto expected_size = strlen(test_string) + prefix_len;

	log_buffer_output.clear();
	logger.critical(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_log_string(log_level_e::critical, test_string));

	log_buffer_output.clear();
	logger.error(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_log_string(log_level_e::error, test_string));

	log_buffer_output.clear();
	logger.warning(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_log_string(log_level_e::warning, test_string));

	log_buffer_output.clear();
	logger.info(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_log_string(log_level_e::info, test_string));

	log_buffer_output.clear();
	logger.debug(test_string);
	CHECK(expected_size == logger.size());
	logger.flush();
	CHECK(log_buffer_output == construct_log_string(log_level_e::debug, test_string));
}
