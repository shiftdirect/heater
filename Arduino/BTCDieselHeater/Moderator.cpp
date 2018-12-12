#include "Moderator.h"

void 
CModerator::shouldSend(bool set)
{
	_bShouldSend = false;
}

bool 
CModerator::shouldSend()
{
	return _bShouldSend;
}

void
CModerator::reset() 
{
  // install invalid values, retain maps (memory defrag reasons)
	for(auto it = fMemory.begin(); it != fMemory.end(); ++it)  it->second = -100;
	for(auto it = iMemory.begin(); it != iMemory.end(); ++it)  it->second = -100;
	for(auto it = cMemory.begin(); it != cMemory.end(); ++it)  it->second = -100;
}

bool
CModerator::shouldSend(const char* name, float value) 
{
	bool retval = true;
	auto it = fMemory.find(name);
  if(it != fMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		fMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}

bool
CModerator::shouldSend(const char* name, int value) 
{
	bool retval = true;
	auto it = iMemory.find(name);
  if(it != iMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		iMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}

bool
CModerator::shouldSend(const char* name, unsigned char value) 
{
	bool retval = true;
	auto it = cMemory.find(name);
  if(it != cMemory.end()) {
		retval = it->second != value;
		it->second = value;
	}
	else {
		cMemory[name] = value;
	}
  _bShouldSend |= retval;
	return retval;
}
