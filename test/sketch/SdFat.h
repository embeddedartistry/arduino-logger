#ifndef SDFAT_TEST_H_
#define SDFAT_TEST_H_

// This file is just to get AVRSDRotationalLogger to compile.
#include <Print.h>

#define DEDICATED_SPI 0
#define SdSpiConfig(a, b) (a)

#define O_WRITE 1
#define O_CREAT 2
#define O_APPEND 3
#define SD_CARD_ERROR_ACMD41 0xff

void printSdErrorSymbol(Print* p, int err)
{

}

class Card {
public:
	unsigned sectorCount()
	{
		return 100;
	}
};

class SdFs {
public:
 void begin(int a)
 {

 }

 int sdErrorCode()
 {
 	return 0;
 }

 unsigned sdErrorData()
 {
 	return 0;
 }

 Card* card()
 {
 	return &c;
 }

private:
 Card c;
};

class FsFile {
public:
	int open(const char* name, int flags)
	{
		return 0;
	}

	int write(char* buffer, size_t size)
	{
		return 0;
	}

	void close()
	{

	}

	void truncate(size_t size)
	{

	}

	unsigned size() const
	{
		return 0;
	}
};

#endif SDFAT_TEST_H_
