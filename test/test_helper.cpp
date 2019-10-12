#include "test_helper.hpp"
#include "printf.h"

std::string log_buffer_output;

void _putchar(char character)
{
	log_buffer_output += character;
}
