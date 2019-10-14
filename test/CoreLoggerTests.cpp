#include <ArduinoLogger.h>
#include <catch.hpp>
#include <string>
#include <test_helper.hpp>

// TODO: add tests for global macros (static inst) - do they do the right thing?

class test
{
  public:
	static std::string check_func_name(void)
	{
		return std::string(FUNC());
	}

	static std::string check_pretty_func_name(void)
	{
		return std::string(PRETTY_FUNC());
	}
};

TEST_CASE("TRACE Macro", "[CoreLogger]")
{
	std::string_view trace_string{TRACE()};
	std::string compare_string = std::string(__SHORT_FILE__) + ":" + std::to_string(__LINE__ - 1);
	CHECK(trace_string == compare_string);
}

TEST_CASE("Func Macro", "[CoreLogger]")
{
	CHECK(std::string_view("check_func_name") == std::string_view(test::check_func_name()));
}

TEST_CASE("Pretty Func Macro", "[CoreLogger]")
{
	CHECK(std::string_view("static std::string test::check_pretty_func_name()") ==
		  std::string_view(test::check_pretty_func_name()));
}

TEST_CASE("Short File Macro", "[CoreLogger]")
{
	CHECK(std::string_view("CoreLoggerTests.cpp") == std::string_view(__SHORT_FILE__));
}

TEST_CASE("Log Level to String", "[CoreLogger]")
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

TEST_CASE("Log Level to Short String", "[CoreLogger]")
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
