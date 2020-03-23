

#include <Preferences.h>

class ABpreferences : public Preferences {
public:
  bool hasBytes(const char* key);
  bool hasString(const char* key);
};