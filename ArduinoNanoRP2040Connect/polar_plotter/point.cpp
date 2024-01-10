#include "point.h"

Point::Point() {
  this->x = 0;
  this->y = 0;
  this->radius = 0;
  this->azimuth = 0;
}

Point::Point(float radius, float azimuth) {
  this->repoint(radius, azimuth);
}

Point::Point(float x, float y, float radius, float azimuth) {
  this->x = x;
  this->y = y;
  this->radius = radius;
  this->azimuth = azimuth;
}

float Point::getX() {
  return this->x;
}

float Point::getY() {
  return this->y;
}

float Point::getRadius() {
  return this->radius;
}

float Point::getAzimuth() {
  return this->azimuth;
}

void Point::repoint(float radius, float azimuth) {
  this->x = radius * cos(azimuth);
  this->y = radius * sin(azimuth);
  this->radius = radius;
  this->azimuth = azimuth;
}