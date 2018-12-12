#ifndef __BTC_MODERATOR_H__
#define __BTC_MODERATOR_H__

#include <map>

class CModerator {
	bool _bShouldSend;
  std::map<const char*, float> fMemory;
  std::map<const char*, int> iMemory;
  std::map<const char*, unsigned char> cMemory;
public:
	void shouldSend(bool reset);
	bool shouldSend();
  bool shouldSend(const char* name, float value);
  bool shouldSend(const char* name, int value);
  bool shouldSend(const char* name, unsigned char value);
	void reset();
};

#endif // __BTC_MODERATOR_H__
