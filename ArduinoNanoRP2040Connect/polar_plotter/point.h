#include "math.h"

class Point {
  private:
    float x;
    float y;
    float radius;
    float azimuth;
  public:
    Point();
    Point(float radius, float azimuth);
    Point(float x, float y, float radius, float azimuth);
    float getX();
    float getY();
    float getRadius();
    float getAzimuth();
    void repoint(float radius, float azimuth);
};
