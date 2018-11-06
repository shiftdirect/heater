
struct sNVStore {
  unsigned char Pmin;
  unsigned char Pmax;
  unsigned short Fmin;
  unsigned short Fmax;
  unsigned char ThermostatMode;
  unsigned char setTemperature;
};

class CNVStorage {
  public:
    CNVStorage() {};
    virtual ~CNVStorage() {};
    virtual void init() = 0;
    virtual void load() = 0;
    virtual void save() = 0;
};


class CHeaterStorage : public CNVStorage {
protected:
  sNVStore calValues;
public:
    CHeaterStorage();
    virtual ~CHeaterStorage() {};

  // TODO: These are only here to allow building without fully 
  // fleshing out NV storage for Due, Mega etc
  // these should be removed once complete (pure virtual)
  void init() {};
  void load() {};
  void save() {};


    unsigned char getPmin();
    unsigned char getPmax();
    unsigned short getFmin();
    unsigned short getFmax();
    unsigned char getTemperature();
    unsigned char getThermostatMode();

    void setPmin(unsigned char val);
    void setPmax(unsigned char val);
    void setFmin(unsigned short val);
    void setFmax(unsigned short val);
    void setTemperature(unsigned char val);
    void setThermostatMode(unsigned char val);
};


#ifdef ESP32

#include <Preferences.h>

class CESP32HeaterStorage : public CHeaterStorage {
  Preferences preferences;
public:
  CESP32HeaterStorage();
  virtual ~CESP32HeaterStorage();
  void init();
  void load();
  void save();
};

#endif

extern CHeaterStorage* pNVStorage;

