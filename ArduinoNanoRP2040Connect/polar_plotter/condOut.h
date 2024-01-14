#include "constants.h"
#include "Arduino.h"
#include <LiquidCrystal.h>
#include "point.h"

class CondOut {
private:
  LiquidCrystal lcd;

public:
  CondOut(LiquidCrystal& lcd);
  void init();

  void print(String&);
  void print(const char[]);
  void print(int, int = DEC);
  void print(unsigned int, int = DEC);
  void print(float, int = DEC);
  void print(double, int = DEC);
  void print(long, int = DEC);
  void print(unsigned long, int = DEC);
  void print(String, Point&);
  void println(String&);
  void println(const char[]);
  void println(int, int = DEC);
  void println(unsigned int, int = DEC);
  void println(float, int = DEC);
  void println(double, int = DEC);
  void println(long, int = DEC);
  void println(unsigned long, int = DEC);
  void println(String, Point&);

  void clear();
  void setCursor(int, int);
  void lcdPrint(String&);
  void lcdPrint(const char[]);
  void lcdPrint(long, int = DEC);
  void lcdPrint(const char[], String&);
  void lcdPrint(const char[], const char[]);
};
