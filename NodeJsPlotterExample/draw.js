const { createCanvas } = require("canvas");
const fs = require("fs");
const GIFEncoder = require("gifencoder");

const CSV_FILE_PATH = './serial.log';
const enableGif = false;
const enablePng = true;
const showLineDebug = false;

const csvLines = fs.readFileSync(CSV_FILE_PATH).toString("utf-8").split('\n');
const positions = csvLines.filter(line => line.length > 0 && line.indexOf(',') > 0 && line.startsWith('POSITION: '))
                      .map(line => {
                          const parts = line.substring(10).split(',');
                          return {radius:parseFloat(parts[0]),azimuth:parseFloat(parts[1])};
                      });
console.log("Positions: " + positions.length);
let width = 1200;
let height = 1200;
let radiusStepSize = 1;
let azimuthStepSize = Math.PI / 100.0;
let marbleSizeInRadiusSteps = 20;
csvLines.filter(line => line.startsWith("Initializing "))
        .forEach(line => {
            const parts = /Initializing ([A-Z_]+): ([+-]?[0-9]+(.[0-9]+)?)/.exec(line);
            if (!parts || parts.length < 3) return;

            if (parts[1] === 'MAX_RADIUS') width = height = parseInt(parts[2]) * 2;
            if (parts[1] === 'RADIUS_STEP_SIZE') radiusStepSize = parseFloat(parts[2]);
            if (parts[1] === 'AZIMUTH_STEP_SIZE') azimuthStepSize = parseFloat(parts[2]);
            if (parts[1] === 'MARBLE_SIZE_IN_RADIUS_STEPS') marbleSizeInRadiusSteps = parseInt(parts[2]);
        });

const marbleSize = marbleSizeInRadiusSteps * radiusStepSize;
width += marbleSize;
height += marbleSize;
const polarOriginX = width / 2;
const polarOriginY = height / 2;
const minimumNumberOfBatchesForLargeDrawing = 40;
const batchSize = positions.length > minimumNumberOfBatchesForLargeDrawing ? Math.floor(positions.length / minimumNumberOfBatchesForLargeDrawing) : 1;
const batchAccumulator = [];
const batches = [];
positions.forEach(step => {
    batchAccumulator.push(step);
    if (batchAccumulator.length == batchSize) {
        batches.push([...batchAccumulator]);
        batchAccumulator.splice(0, batchAccumulator.length);
    }
});
if (batchAccumulator.length > 0) {
    batches.push([...batchAccumulator]);
}

if (enableGif) {
    const encoder = new GIFEncoder(width, height);
    encoder.start();
    encoder.setRepeat(0);
    encoder.setDelay(500);
    encoder.setQuality(10);

    for (let i = -1; i < batches.length; i++) {
        console.log("FRAME " + (i + 1) + " of " + batches.length);

        const frame = getImage(i);

        encoder.addFrame(frame[1]);
    }

    encoder.finish();

    const buffer = encoder.out.getData();
    fs.writeFileSync("./image.gif", buffer);
    console.log("Wrote animated GIF to image.gif");
}

if (enablePng) {
    const finalImage = getImage(batches.length - 1, showLineDebug)[0];
    const imageBuffer = finalImage.toBuffer("image/png");
    fs.writeFileSync("./image.png", imageBuffer);
    console.log("Write final image to image.png");
}

function getImage(maxBatch, debugLines) {
    const canvas = createCanvas(width, height);
    const context = canvas.getContext("2d");
    const maxRadius = Math.min(width, height) / 2;

    context.fillStyle = "#ffffff";
    context.fillRect(0, 0, width, height);
    context.translate(polarOriginX, polarOriginY);

    context.strokeStyle = "#000000";
    context.lineCap = "round";
    context.lineWidth = marbleSize * 2 / 3;
    context.beginPath();
    context.moveTo(0, 0);

    for (let j = 0; j <= maxBatch; j++) {
        const batch = batches[j];
        console.log("Drawing frame " + (j + 1) + " of " + batches.length + " with " + batch.length + " positions");
        batch.forEach(position => {
            if (position.radius === undefined || position.azimuth === undefined) return;

            const x = position.radius * Math.cos(position.azimuth);
            const y = position.radius * Math.sin(position.azimuth);
            console.log("    (R: " + position.radius + ", A: " + position.azimuth + " ), (X: " + x + ", Y: " + y + " )");
            context.lineTo(x, y);
        });
        context.stroke();
    }

    context.lineWidth = 2;
    context.beginPath();
    context.arc(0, 0, maxRadius, 0, 2 * Math.PI);
    context.moveTo(0, -1 * maxRadius);
    context.lineTo(0, -1 * maxRadius + 8);
    context.stroke();

    return [canvas, context];
}