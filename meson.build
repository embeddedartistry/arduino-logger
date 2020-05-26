project('Arduino SDK Logger',
	['c', 'cpp'],
	default_options : [
		# `build.*` options affect `native: true targets`
		# plain options affect `native: false` targets.
		'cpp_std=c++11', 'build.cpp_std=c++11'
	],
	license: 'MIT',
	meson_version: '>0.54.0',
	version: '1.0')

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

#######################
# Process source tree #
#######################

# Add files to this variable if you want them analyzed by clang-tidy
clangtidy_files = [
	files('src/ArduinoLogger.cpp')
]

test_includes = [
	include_directories('test'),
	include_directories('src'),
]

logging_tests = executable('arduino_logger_tests',
	[
		files('src/ArduinoLogger.cpp'),
		files('test/CircularBufferLoggerTests.cpp'),
		# Currently disabled due to use of AVR header
		#files('test/AVRCircularBufferLoggerTests.cpp'),
		files('test/catch_main.cpp'),
		files('test/printf.c'),
		files('test/test_helper.cpp'),
		files('test/CoreLoggerTests.cpp'),
	],
	include_directories: test_includes,
	native: true,
)

test('ArduinoLogger_tests',
	logging_tests)

############################
# Supporting Build Targets #
############################

clangformat_excludes = [
	meson.source_root() / 'test/catch.hpp',
	meson.source_root() / 'test/printf.c'
]

subdir('build/analysis/clang-tidy')
subdir('build/analysis/complexity')
subdir('build/analysis/cppcheck')
subdir('build/format')
