#include "drawingProducer.h"

class KnownDrawingProducer : public DrawingProducer {
private:
  int drawingIndex;
  String nextDrawing;

protected:
  String* getNextDrawing();
  using DrawingProducer::setDrawingName;

public:
  using DrawingProducer::tryGetNewDrawing;
  using DrawingProducer::getDrawing;
  using DrawingProducer::getCommandCount;
  using DrawingProducer::getCommand;
};
