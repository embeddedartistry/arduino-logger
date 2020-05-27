project('Arduino SDK Logger',
	['c', 'cpp'],
	default_options : [
		'buildtype=minsize',
		'b_lto=true',
		'b_staticpic=false',
		# `build.*` options affect `native: true targets`
		# plain options affect `native: false` targets.
		'cpp_std=gnu++11', 'build.cpp_std=gnu++11'
    ],
	license: 'MIT',
	meson_version: '>0.54.0',
	version: '1.0'
)

# TODO: These sketches are currently not built, they're targeted for Teensy (ARM)
# CircularLogBuffer
# CircularLogBuffer_CompileTimeFiltering
# CircularLogBuffer_GlobalInst
# CircularLogBuffer_LocalInst
# SDFileLogger_Teensy.cpp
# TeensySDLogger.cpp
# TeensySDRotationalLogger.cpp

########################
# Identify Compiler(s) #
########################

subdir('build/compiler')
subdir('build/compiler/c')
subdir('build/compiler/cpp')

if get_option('enable-werror') == true
	desired_common_compile_flags += '-Werror'
endif

if get_option('enable-pedantic-error') == true
	desired_common_compile_flags += '-pedantic-error'
endif

compile_settings_list = [
	{'lang': 'cpp', 'compiler': host_cpp_compiler, 'flags': desired_cpp_compile_flags, 'isnative': false},
	{'lang': 'cpp', 'compiler': native_cpp_compiler, 'flags': desired_native_cpp_compile_flags, 'isnative': true},
]

# Process the compilation flags
subdir('build/compiler/check-and-apply-flags')

arduinoprintf = subproject('arduino-printf')
libPrintf_test_dep = arduinoprintf.get_variable('libPrintf_test_dep')

#######################
# Process source tree #
#######################

if meson.is_cross_build()
	if host_machine.cpu_family() != 'avr'
		error('Cross builds for this library are only enabled for AVR targets')
	endif

	# This library uses `static`, which is not defined for avr-libc
	add_project_arguments('-fno-threadsafe-statics', language: 'cpp', native: false)

	libPrintf_dep = arduinoprintf.get_variable('libPrintf_dep')

	arduinocore = subproject('arduinocore-avr')
	arduinocore_dep = arduinocore.get_variable('arduinocore_dep')
	arduinocore_main_dep = arduinocore.get_variable('arduinocore_main_dep')
	arduinocore_eeprom_dep = arduinocore.get_variable('arduinocore_eeprom_dep')

	arduinoprintf = subproject('arduino-printf')
	libPrintf_dep = arduinoprintf.get_variable('libPrintf_dep')

	logger_includes = include_directories('src')
	LibArduinoLogger = static_library('ArduinoLogger',
	    'src/ArduinoLogger.cpp',
	   	include_directories: logger_includes,
	    dependencies: [
	    	arduinocore_dep,
	    	libPrintf_dep,
	    ],
	    build_by_default: meson.is_subproject() == false,
	)

	libArduinoLogger_dep = declare_dependency(
		include_directories: logger_includes,
		link_with: LibArduinoLogger,
		dependencies: libPrintf_dep,
	)

	if meson.is_subproject() == false
		executable('AVRCircularLogBuffer',
			files('test/sketch/AVRCircularLogBuffer.cpp', 'test/sketch/Adafruit_SleepyDog.cpp'),
			include_directories: include_directories('test', 'test/sketch', is_system: true),
			dependencies: [
				libArduinoLogger_dep,
				arduinocore_main_dep,
			],
			install: false,
			build_by_default: meson.is_subproject() == false,
		)

		executable('AVRSDRotationalLogger-donotuse',
			files('test/sketch/AVRSDRotationalLogger.cpp', 'test/sketch/Adafruit_SleepyDog.cpp'),
			include_directories: include_directories('test', 'test/sketch', is_system: true),
			dependencies: [
				libArduinoLogger_dep,
				arduinocore_main_dep,
				arduinocore_eeprom_dep
			],
			install: false,
			build_by_default: meson.is_subproject() == false,
		)
	endif
endif

# Add files to this variable if you want them analyzed by clang-tidy
clangtidy_files = [
	files('src/ArduinoLogger.cpp')
]

################
# Test Targets #
################

logging_tests = executable('arduino_logger_tests',
	[
		files('src/ArduinoLogger.cpp'),
		files('test/CircularBufferLoggerTests.cpp'),
		# Currently disabled due to use of AVR header
		#files('test/AVRCircularBufferLoggerTests.cpp'),
		files('test/catch_main.cpp'),
		files('test/test_helper.cpp'),
		files('test/CoreLoggerTests.cpp'),
	],
	include_directories: include_directories('test', 'test/catch', 'src'),
	dependencies: libPrintf_test_dep,
	native: true,
	build_by_default: meson.is_subproject() == false,
)

if meson.is_subproject() == false
	test('ArduinoLogger_tests',
		logging_tests)
endif

############################
# Supporting Build Targets #
############################

clangformat_excludes = [
	meson.source_root() / 'test/catch',
	meson.source_root() / 'test/sketch',
]

cppcheck_paths = [
	meson.source_root() / 'src'
]

subdir('build/analysis/clang-tidy')
subdir('build/analysis/complexity')
subdir('build/analysis/cppcheck')
subdir('build/format')
