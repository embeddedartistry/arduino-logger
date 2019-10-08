#include <catch.hpp>
#include <ArduinoLogger.h>

TEST_CASE("EmptyTestCase", "[ArduinoLogger]")
{
	// Force a failure so we can see everything working
	REQUIRE(true == false);
}
