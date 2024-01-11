#include "stepBank.h"
#include "point.h"

class LineStepCalculator {
private:
  unsigned int debugLevel;
  Point middle;
  Point keepMinorIncrementMajor;
  Point incrementMinorIncrementMajor;
  Point incrementMinorKeepMajor;
  float radiusStepSize;
  float azimuthStepSize;

  void addBulkSteps(StepBank& steps, float radiusStepOffset, float azimuthStepOffset, int stepCount) {
    int radiusStep = radiusStepOffset > 0.0000001 ? 1 : (radiusStepOffset < -0.0000001 ? -1 : 0);
    int azimuthStep = azimuthStepOffset > 0.0000001 ? 1 : (azimuthStepOffset < -0.0000001 ? -1 : 0);

    for (int i = 0; i < stepCount; i++) {
      steps.addStep(radiusStep, azimuthStep);
    }
  }

  void addCompoundStepsForLine(StepBank& steps, Point& start, Point& finish, float startA) {
    Point kpMinIncMajP = this->keepMinorIncrementMajor;
    Point incMinIncMajP = this->incrementMinorIncrementMajor;
    Point incMinKpMajP = this->incrementMinorKeepMajor;

    float xDelta = start.getX() - finish.getX();
    float yDelta = finish.getY() - start.getY();
    float distanceDenominator = sqrt(xDelta * xDelta + yDelta * yDelta);
    float radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
    float azimuthStepOffset = finish.getAzimuth() > startA ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
    int radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
    int azimuthSteps = round(abs(finish.getAzimuth() - startA) / this->azimuthStepSize);
    bool moreRadiusSteps = radiusSteps > azimuthSteps;
    int majorSteps = moreRadiusSteps ? radiusSteps : azimuthSteps;
    int minorStep = 0;

    if (this->debugLevel >= 2) {
      Serial.print("  Compound steps from (");
      Serial.print(start.getX(), 4);
      Serial.print(",");
      Serial.print(start.getY(), 4);
      Serial.print(") to (");
      Serial.print(finish.getX(), 4);
      Serial.print(",");
      Serial.print(finish.getY(), 4);
      Serial.print("), radiusSteps=");
      Serial.print(radiusSteps);
      Serial.print(", azimuthSteps=");
      Serial.print(azimuthSteps);
      Serial.print(", radiusStepOffset=");
      Serial.print(radiusStepOffset, 4);
      Serial.print(", azimuthStepOffset=");
      Serial.print(azimuthStepOffset);
      Serial.print(", xDelta=");
      Serial.print(xDelta, 4);
      Serial.print(", yDelta=");
      Serial.print(yDelta, 4);
      Serial.print(", distanceDenominator=");
      Serial.println(distanceDenominator, 4);
    }

    for (int majorStep = 0; majorStep <= majorSteps;) {
      float radius = start.getRadius() + (moreRadiusSteps ? majorStep : minorStep) * radiusStepOffset;
      float azimuth = startA + (moreRadiusSteps ? minorStep : majorStep) * azimuthStepOffset;

      if (moreRadiusSteps) {
        kpMinIncMajP.repoint(radius + radiusStepOffset, azimuth);
        incMinIncMajP.repoint(radius + radiusStepOffset, azimuth + azimuthStepOffset);
        incMinKpMajP.repoint(radius, azimuth + azimuthStepOffset);
      } else {
        kpMinIncMajP.repoint(radius, azimuth + azimuthStepOffset);
        incMinIncMajP.repoint(radius + radiusStepOffset, azimuth + azimuthStepOffset);
        incMinKpMajP.repoint(radius + radiusStepOffset, azimuth);
      }

      float kpMinIncMajDist = findDistanceFromLine(start, kpMinIncMajP, xDelta, yDelta, distanceDenominator);
      float incMinIncMajDist = findDistanceFromLine(start, incMinIncMajP, xDelta, yDelta, distanceDenominator);
      float incMinKpMajDist = findDistanceFromLine(start, incMinKpMajP, xDelta, yDelta, distanceDenominator);
      if (this->debugLevel >= 3) {
        Serial.print("   KeepMinorIncrementMajorPoint=(");
        Serial.print(kpMinIncMajP.getX(), 4);
        Serial.print(", ");
        Serial.print(kpMinIncMajP.getY(), 4);
        Serial.print(", ");
        Serial.print(kpMinIncMajP.getRadius(), 4);
        Serial.print(", ");
        Serial.print(kpMinIncMajP.getAzimuth(), 4);
        Serial.print("; IncrementMinorIncrementMajorPoint=(");
        Serial.print(incMinIncMajP.getX(), 4);
        Serial.print(", ");
        Serial.print(incMinIncMajP.getY(), 4);
        Serial.print(", ");
        Serial.print(incMinIncMajP.getRadius(), 4);
        Serial.print(", ");
        Serial.print(incMinIncMajP.getAzimuth(), 4);
        Serial.print("; IncrementMinorKeepMajorPoint=(");
        Serial.print(incMinKpMajP.getX(), 4);
        Serial.print(", ");
        Serial.print(incMinKpMajP.getY(), 4);
        Serial.print(", ");
        Serial.print(incMinKpMajP.getRadius(), 4);
        Serial.print(", ");
        Serial.println(incMinKpMajP.getAzimuth(), 4);
      }

      if (incMinIncMajDist <= incMinKpMajDist && incMinIncMajDist < kpMinIncMajDist) {
        minorStep++;
        majorStep++;
        addStep(steps, radiusStepOffset, 1, azimuthStepOffset, 1);
      } else if (incMinKpMajDist < incMinIncMajDist && incMinKpMajDist < kpMinIncMajDist) {
        minorStep++;
        addStep(steps, radiusStepOffset, moreRadiusSteps ? 0 : 1, azimuthStepOffset, moreRadiusSteps ? 1 : 0);
      } else {
        majorStep++;
        addStep(steps, radiusStepOffset, moreRadiusSteps ? 1 : 0, azimuthStepOffset, moreRadiusSteps ? 0 : 1);
      }

      if (this->debugLevel >= 4) {
        Serial.print("    KeepMinorIncrementMajorDistance=");
        Serial.print(kpMinIncMajDist, 4);
        Serial.print(", IncrementMinorIncrementMajorDistance=");
        Serial.print(incMinIncMajDist, 4);
        Serial.print(", IncrementMinorKeepMajorDistance=");
        Serial.print(incMinKpMajDist, 4);
        Serial.print(", MinorStep=");
        Serial.print(minorStep);
        Serial.print(", MajorStep=");
        Serial.println(majorStep);
      }
    }
  }

  void addStep(StepBank& steps, float radiusStepOffset, int radiusStepIncrement, float azimuthStepOffset, int azimuthStepIncrement) {
    int radiusStep = getStepValue(radiusStepOffset, radiusStepIncrement);
    int azimuthStep = getStepValue(azimuthStepOffset, azimuthStepIncrement);

    if (this->debugLevel >= 4) {
      Serial.print("    RadiusStepIncrement=");
      Serial.print(radiusStepIncrement, 4);
      Serial.print(", RadiusStep=");
      Serial.print(radiusStep, 4);
      Serial.print(", AzimuthStepIncrement=");
      Serial.print(azimuthStepIncrement, 4);
      Serial.print(", AzimuthStep=");
      Serial.println(azimuthStep, 4);
    }

    steps.addStep(radiusStep, azimuthStep);
  }

  int getStepValue(float offset, int increment) {
    if ((offset > 0.0000001 && increment > 0) || (offset < -0.0000001 && increment < 0)) {
      return 1;
    }
    if ((offset > 0.0000001 && increment < 0) || (offset < -0.0000001 && increment > 0)) {
      return -1;
    }

    return 0;
  }

  float findDistanceToFinish(Point& finish, Point& point) {
    float xDelta = finish.getX() - point.getX();
    float yDelta = finish.getY() - point.getY();
    return sqrt(xDelta * xDelta + yDelta * yDelta);
  }

  float findDistanceFromLine(Point& start, Point& point, float xDelta, float yDelta, float distanceDenominator) {
    return abs(xDelta * (point.getY() - start.getY()) - (start.getX() - point.getX()) * yDelta) / distanceDenominator;
  }

  void orientEndPoint(Point& start, Point& end) {
    float startA = start.getAzimuth();
    float startX = start.getX();
    float fullCircle = PI * 2;
    float endA = end.getAzimuth();
    float endX = end.getX();

    float slope = (end.getY() - start.getY()) / (endX - startX);
    float yIntercept = end.getY() - slope * endX;
    float xIntercept = -1 * yIntercept / slope;

    // Ensure we sweep the correct direction given the beginning and ending points of the line
    if ((xIntercept < 0 && yIntercept < 0 && startX > endX)
        || (xIntercept < 0 && yIntercept > 0 && startX < endX)
        || (xIntercept > 0 && yIntercept < 0 && startX > endX)
        || (xIntercept > 0 && yIntercept > 0 && startX < endX)) {
      while (endA > startA) {
        endA -= fullCircle;
      }
    } else if ((xIntercept < 0 && yIntercept < 0 && startX < endX)
               || (xIntercept < 0 && yIntercept > 0 && startX > endX)
               || (xIntercept > 0 && yIntercept < 0 && startX < endX)
               || (xIntercept > 0 && yIntercept > 0 && startX > endX)) {
      while (endA < startA) {
        endA += fullCircle;
      }
    }

    end.repoint(end.getRadius(), endA);
  }

public:
  LineStepCalculator();
  void init(float radiusStepSize, float azimuthStepSize);
  void addLineSteps(Point& start, Point& finish, StepBank& steps);
  void setDebug(unsigned int level);
};
