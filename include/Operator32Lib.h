#ifndef OP32LIB
#define OP32LIB

#include <windows.h>
#include <iostream>
#include <string>
#include <psapi.h>
#include "TlHelp32.h"

using namespace std;

class Operator32Lib
{
public:
	static uint32_t UnsignedShift32(uint32_t base, uint32_t shift)
	{
		return base << shift;
	}
};

#endif
