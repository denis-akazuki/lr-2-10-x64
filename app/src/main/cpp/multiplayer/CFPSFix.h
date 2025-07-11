#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <list>

class CFPSFix
{
private:
	[[noreturn]] void Routine();

public:
	CFPSFix();
	~CFPSFix()= default;
};

