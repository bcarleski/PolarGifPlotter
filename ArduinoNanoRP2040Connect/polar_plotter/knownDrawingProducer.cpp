#include "knownDrawingProducer.h"

const int KNOWN_DRAWINGS_COUNT = 3;
const String KNOWN_DRAWINGS[] = {
  "{\"commands\":[\"L300,300\",\"L300,-300\",\"L-300,-300\",\"L-300,300\",\"L300,300\"]}",
  "{\"commands\":[\"L0,170\",\"L-120,-170\",\"L180,70\",\"L-180,70\",\"L120,-170\",\"L0,170\"]}",
  "{\"commands\":[\"L0,200\",\"C-136,100,180\",\"L0,-256\",\"L272,0\",\"C136,100,180\"]}",
};

String* KnownDrawingProducer::getNextDrawing() {
  int idx = drawingIndex % KNOWN_DRAWINGS_COUNT;
  drawingIndex++;

  String drawingName = "Drawing_" + String(idx + 1);
  setDrawingName(drawingName);

  nextDrawing = KNOWN_DRAWINGS[idx];
  return &nextDrawing;
}
