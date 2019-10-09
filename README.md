# arduino-logger

Flexible logging library for the Arduino SDK. This library allows the same logging interface to be used with multiple outputs. Users are able to define their own logging strategies specific to their system.

**Table of Contents:**

1. [Dependencies](#dependencies)
2. [Logging Basics](#logging-basics)
3. [Using the Library](#using-the-library)
    1. [Provided Logging Implementations](#provided-logging-implementations)
    2. [Selecting a Logging Strategy](#selecting-a-logging-strategy)
4. [Compile-time Configuration](#compile-time-configuration)
    1. [Maximum Log Level](#maximum-log-level)
    2. [Log Name Strings](#log-name-strings)
    3. [Echo to Serial](#echo-to-serial)
5. [Run-time Configuration](#run-time-configuration)
6. [Examples](#examples)
7. [Creating a Custom Logging Strategy](#creating-a-custom-logging-strategy)
8. [Tests](#tests)

## Dependencies

This library requires the following Arduino library to be installed on your system:

* [embeddedartistry/arduino-printf](https://github.com/embeddedartistry/arduino-printf)

Some pre-built logging classes will depend on other libraries to be installed, such as:

* [greiman/SdFat](https://github.com/greiman/SdFat)

## Logging Basics

This library provides multiple logging levels:

* Critical
* Error
* Warning
* Debug
* Off

This library is designed so that a single static logger instance is used across the application. If you have multiple files, the same logging instance will be used in all files as long as you use the provided logging macros.

Log levels are configurable at compile-time and run-time. 

When a log level is set during compile-time, any levels rated "lower" in priority will be excluded. For example, setting the compile-time log level to "critical" will cause "error", "warning", and "debug" log statements to be removed from the binary.

When a log level is set during run-time, log statements lower in priority will remain in the binary but will be ignored by the logger.

## Using the Library

To use this library, you will need to [create a header which selects or defines a logging strategy](#selecting-a-logging-strategy). This header must be included in your application.

To interact with the logger, use the following macros to ensure you are accessing the same static logger instance:

* `logdebug()`
* `loginfo()`
* `logwarning()`
* `logerror()`
* `logcritical()`

These macros accept `printf`-like format strings. All `printf` format codes are supported:

```
loginfo("Loop iteration %d\n", iterations);
```

If the selected logging implementation requires that the program logic controls when to flush the log buffer to the target output (such as the Circular Log Buffer), use the `logflush()` macro:

```
  // This code will flush to serial every 10 loop iterations
  if((iterations % 10) == 0)
  {
    printf("Log buffer contents:\n");
    logflush();
  }
```

You can clear all contents from the log buffer using `logclear()`. This will empty the buffer, but will not flush the contents to the output.

### Provided Logging Implementations

* [Circular Log Buffer](CircularBufferLogger.h)
    - Log information is stored in a circular buffer in RAM
    - When the buffer is full, old data is overwritten with new data
    - Ability to print all log buffer information over the `Serial` device
* [AVR-specialized Circular Buffer](AVRCircularBufferLogger.h)
    - Log information is stored in a circular buffer in RAM
    - When the buffer is full, old data is overwritten with new data
    - Ability to print all log buffer information over the `Serial` device
* [SD Logger](SDCardLogger.h)
    - Writes log information to an SD card slot
    - Uses the [SdFat](https://github.com/greiman/SdFat) Library

### Selecting a Logging Strategy

In your project, you will need to create a `platform_logger.h` file (or pick another name of your choosing). Within the file, you need two items:

1. An `#include` directive which selects the proper header
2. A `using` statement which defines `PlatformLogger` appropriately for your class
    * A `PlatformLogger_t` templated wrapper type is used to ensure a single static logging instance is provided
    * All logging macros depend on the `using` statement being constructed properly

Here is an example definition of the `platform_logger.h` file. This file uses the `AVRCircularBufferLogger` class, which takes a template parameter specifying a buffer size of 1024 B.

```
#ifndef PLATFORM_LOGGER_HPP_
#define PLATFORM_LOGGER_HPP_

#include <AVRCircularBufferLogger.h>

using PlatformLogger =
    PlatformLogger_t<AVRCircularLogBufferLogger<1024>>;

#endif
```

## Compile-Time Configuration

### Maximum Log Level

By default, all log statements are compiled into a binary. To change the compile-time log level and filter out unwanted messages during compilation, you need to set the `LOG_LEVEL` macro to the desired setting:

```
/// Logging is disabled
#define LOG_LEVEL_OFF 0
/// Indicates the system is unusable, or an error that is unrecoverable
#define LOG_LEVEL_CRITICAL 1
/// Indicates an error condition
#define LOG_LEVEL_ERROR 2
/// Indicates a warning condition
#define LOG_LEVEL_WARNING 3
/// Informational messages
#define LOG_LEVEL_INFO 4
/// Debug-level messages
#define LOG_LEVEL_DEBUG 5
```

One approach is to define the `LOG_LEVEL` macro in your build system. 

For example, this will set the compile-time log level to "warning", removing "info" and "debug" messages from the build.

```
-DLOG_LEVEL=3
```

You can also define the `LOG_LEVEL` macro in your `platform_logger.h` file, before including the necessary logging class header:

```
// This will change the compile-time log level to compile-out logdebug messages
#define LOG_LEVEL LOG_LEVEL_INFO
#include <CircularBufferLogger.h>
```

### Log Name Strings

You can re-define the `LOG_LEVEL_NAMES` and `LOG_LEVEL_SHORT_NAMES` macros to provide your own string name definitions for each logging level.

These settings can be changed in the build system, but it is easiest to define them in `platform_logger.h` before including the necessary logging library header.

### Echo to Serial

By default, the logging library does not echo logging calls to the serial console. You can change the default setting at compile-time using the `LOG_ECHO_EN_DEFAULT` definition.

The logging library will print the output using the `printf` function. To change that output, see the instructions in the [embeddedartistry/arduino-printf](https://github.com/embeddedartistry/arduino-printf) library.

The setting can be changed in your build system or by defining the desired value in `platform_logger.h` before including the necessary logging library header.

```
-DLOG_ECHO_EN_DEFAULT=true
```

```
#define LOG_ECHO_EN_DEFAULT true
#include <CircularBufferLogger.h>
```

### Disable All Logging Calls

You can remove all logging calls from the binary at compile-time by defining `LOG_EN_DEFAULT` to `false`. 


The setting can be changed in your build system or by defining the desired value in `platform_logger.h` before including the necessary logging library header.

```
-DLOG_EN_DEFAULT=false
```

```
#define LOG_EN_DEFAULT false
#include <CircularBufferLogger.h>
```

## Run-Time Configuration

You can control the run-time logging level using the `loglevel()` macro. This will tell the logging library to filter out levels below the specified priority level.

```
loglevel(log_level_e::info)
```

You can control the echo-to-console behavior during runtime with the `logecho()` macro. This will tell the logging library to enable/disable printing logging calls via `printf()`.

```
logecho(true); // enables echoing via printf()
```

## Examples

* [CircularLogBuffer](examples/CircularLogBuffer)
    - Demonstrates the use of the CicularLogBuffer class in a sketch with a proper "platform_logger.h" header
* [AVRCircularLogBuffer](examples/AVRCircularLogBuffer)
    - Demonstrates the use of the AVRCicularLogBuffer class in a sketch with a proper "platform_logger.h" header
    - This class can be used with AVR devices in the Arduino environment
* [CircularLogBuffer_CompileTimeFiltering](examples/CircularLogBuffer_CompileTimeFiltering)
    - Demonstrates defining `LOG_LEVEL` in `platform_logging.h` to filter out messages during compilation

## Creating a Custom Logging Strategy

TODO

## Tests

Some classes have the ability to be used on a host machine outside of the Arduino SDK.

To run the tests, you will need:

* [meson](http://mesonbuild.com)
* [ninja](https://ninja-build.org)

You can run `make test` to compile and run the tests. You will see generic pass/fail output:

```
1/1 ArduinoLogger_tests                     OK       0.02 s

Ok:                    1
Expected Fail:         0
Fail:                  0
Unexpected Pass:       0
Skipped:               0
Timeout:               0

Full log written to /Users/pjohnston/src/ea/arduino-logger/buildresults/meson-logs/testlog.txt
```

To see more information about a failure, run the binary manually:

```
arduino-logger $ ./buildresults/arduino_logger_tests

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
arduino_logger_tests is a Catch v2.9.2 host application.
Run with -? for options

-------------------------------------------------------------------------------
Create a logger
-------------------------------------------------------------------------------
../test/ArduinoLoggerTests.cpp:12
...............................................................................

../test/ArduinoLoggerTests.cpp:22: FAILED:
  CHECK( LOG_LEVEL_LIMIT() != level )
with expansion:
  5 != 5

===============================================================================
test cases:  5 |  4 passed | 1 failed
assertions: 23 | 22 passed | 1 failed
```
