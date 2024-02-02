#!/bin/bash

cd "$(dirname $0)"
EXAMPLES_DIR="$(pwd)/examples"
PLOTTER_DIR="$(pwd)/NodeJsPlotterExample"
DRAWER_CMD=~/Documents/Arduino/libraries/PolarPlotterCore/runtests
cd - &>/dev/null

if [ -n "$1" ]
then
    DRAWINGS=("$@")
else
    DRAWINGS=($(cat "$EXAMPLES_DIR/drawings.json" | jq -c '.drawings | map(.drawing)' | sed 's/\[//g' | sed 's/\]//g' | sed 's/","/ /g' | sed 's/"//g'))
fi

cd "$PLOTTER_DIR"
for DRAWING in "${DRAWINGS[@]}"
do
    CMDS=($(cat "$EXAMPLES_DIR/$DRAWING.json" | jq -c '.commands' | sed 's/\[//g' | sed 's/\]//g' | sed 's/","/ /g' | sed 's/"//g'))
    echo "${CMDS[@]}" > serial.log
    $DRAWER_CMD 'D9' "${CMDS[@]}" >> serial.log
    cp serial.log "../$DRAWING.log"
    npm run draw > /dev/null
    cp image.png "../$DRAWING.png"
done

cd - &>/dev/null
