#define MAX_STEPS 8192
#include "step.h"
#include "point.h"

class LineSteps {
  private:
    Step steps[MAX_STEPS];
    unsigned int stepCount;
    Point start;
    Point finish;
    float xDelta;
    float yDelta;
    float distanceDenominator;
    float radiusStepSize;
    float azimuthStepSize;

    void addLineSteps() {
      Point start = this->start;
      Point finish = this->finish;
      float startA = abs(start.getRadius()) <= 0.00001 ? finish.getAzimuth() : start.getAzimuth();
      int radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
      int azimuthSteps = round(abs(finish.getAzimuth() - start.getAzimuth()) / this->azimuthStepSize);

      if (radiusSteps == 0 && azimuthSteps == 0) {
        return;
      }

      bool moreRadiusSteps = radiusSteps > azimuthSteps;
      int majorSteps = moreRadiusSteps ? radiusSteps : azimuthSteps;
      float radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
      float azimuthStepOffset = finish.getAzimuth() > start.getAzimuth() ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
      Point* keepMinorPoint = new Point();
      Point* incrementMinorPoint = new Point();
      Point* decrementMinorPoint = new Point();
      int minorStep = 0;

      for (int majorStep = 1; majorStep <= majorSteps; majorStep++) {
        float radius = start.getRadius() + (moreRadiusSteps ? majorStep : minorStep) * radiusStepOffset;
        float azimuth = start.getAzimuth() + (moreRadiusSteps ? minorStep : majorStep) * azimuthStepOffset;
        keepMinorPoint->repoint(radius, azimuth);

        if (moreRadiusSteps) {
          incrementMinorPoint->repoint(radius, azimuth + azimuthStepOffset);
          decrementMinorPoint->repoint(radius, azimuth + azimuthStepOffset);
        } else {
          incrementMinorPoint->repoint(radius + radiusStepOffset, azimuth);
          decrementMinorPoint->repoint(radius - radiusStepOffset, azimuth);
        }

        float keepDistance = findDistanceToPoint(*keepMinorPoint);
        float incrementDistance = findDistanceToPoint(*incrementMinorPoint);
        float decrementDistance = findDistanceToPoint(*decrementMinorPoint);

        if (incrementDistance <= decrementDistance && incrementDistance < keepDistance) {
          minorStep++;
          this->steps[this->stepCount++] = *(new Step(1, 1));
        } else if (decrementDistance < incrementDistance && decrementDistance < keepDistance) {
          minorStep--;
          this->steps[this->stepCount++] = *(new Step(moreRadiusSteps ? 1 : -1, moreRadiusSteps ? -1 : 1));
        } else {
          this->steps[this->stepCount++] = *(new Step(moreRadiusSteps ? 1 : 0, moreRadiusSteps ? 0 : 1));
        }
      }

      delete keepMinorPoint;
      delete incrementMinorPoint;
      delete decrementMinorPoint;
    }

    Point* getCommandPoint(String& command) {
      int commaIndex = command.indexOf(',');
      float rawX = command.substring(1, commaIndex).toFloat();
      float rawY = command.substring(commaIndex + 1).toFloat();
      float rawR = sqrt(rawX * rawX + rawY * rawY);
      float rawA = atan2(rawY, rawX);

      Point start = this->start;
      float startA = start.getAzimuth();
      float startX = start.getX();
      float fullCircle = PI * 2;

      float slope = (rawY - start.getY()) / (rawX - startX);
      float yIntercept = rawY - slope * rawX;
      float xIntercept = -1 * yIntercept / slope;

      // Ensure we sweep the correct direction given the beginning and ending points of the line
      if (   (xIntercept < 0 && yIntercept < 0 && startX > rawX)
          || (xIntercept < 0 && yIntercept > 0 && startX < rawX)
          || (xIntercept > 0 && yIntercept < 0 && startX > rawX)
          || (xIntercept > 0 && yIntercept > 0 && startX < rawX)) {
        while (rawA > startA) {
          rawA -= fullCircle;
        }
      } else if ((xIntercept < 0 && yIntercept < 0 && startX < rawX)
          ||   (xIntercept < 0 && yIntercept > 0 && startX > rawX)
          ||   (xIntercept > 0 && yIntercept < 0 && startX < rawX)
          ||   (xIntercept > 0 && yIntercept > 0 && startX > rawX)) {
        while (rawA < startA) {
          rawA += fullCircle;
        }
      }

      return getPoint(rawR, rawA);
    }

    Point* getPoint(float radius, float azimuth) {
      float r = round(radius / this->radiusStepSize) * this->radiusStepSize;
      float a = round(azimuth / this->azimuthStepSize) * this->azimuthStepSize;
      return new Point(r, a);
    }

    float findDistanceToPoint(Point& point) {
      return abs(this->xDelta * (this->start.getY() - point.getY()) - (this->start.getX() - point.getX()) * this->yDelta) / this->distanceDenominator;
    }

  public:
    LineSteps();
    LineSteps(String& command, Point& current, float radiusStepSize, float azimuthStepSize);
    Step getStep(unsigned int stepIndex);
    unsigned int getStepCount();
};
