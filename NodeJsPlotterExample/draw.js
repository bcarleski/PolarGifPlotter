const { createCanvas } = require("canvas");
const fs = require("fs");
const GIFEncoder = require("gifencoder");

const CSV_FILE_PATH = './serial.log';
const enableGif = false;
const enablePng = true;
const showLineDebug = false;

const csvLines = fs.readFileSync(CSV_FILE_PATH).toString("utf-8").split('\n');
const steps = [];
csvLines.filter(line => line.length > 0 && line.indexOf(',') > 0 && line.startsWith('STEP: '))
        .forEach(line => {
            const parts = line.substring(6).split(',');
            const radiusStep = parseInt(parts[0]);
            const azimuthStep = parseInt(parts[1]);

            if ((radiusStep < -1 || radiusStep > 1) && (azimuthStep < -1 || azimuthStep > 1)) {
                const rSteps = Math.abs(radiusStep);
                const aSteps = Math.abs(azimuthStep);

                if (rSteps == aSteps) {
                    for (let i = 0; i < rSteps; i++) steps.push({radiusStep: 1, azimuthStep: 1});
                } else if (rSteps > aSteps) {
                    const rStepsPerAStep = rSteps / parseFloat(aSteps + 1);
                    for (let i = 0; i < aSteps; i++) steps.push({radiusStep: (radiusStep > 0 ? 1 : -1) * parseInt(Math.round(rStepsPerAStep * (i + 1)) - Math.round(rStepsPerAStep * i)), azimuthStep: azimuthStep > 0 ? 1 : -1});
                    steps.push({radiusStep: (radiusStep > 0 ? 1 : -1) * parseInt(Math.round(rStepsPerAStep * (aSteps + 1)) - Math.round(rStepsPerAStep * aSteps)), azimuthStep: 0});
                } else {
                    const aStepsPerRStep = aSteps / parseFloat(rSteps + 1);
                    for (let i = 0; i < rSteps; i++) steps.push({radiusStep: radiusStep > 0 ? 1 : -1, azimuthStep: (azimuthStep > 0 ? 1 : -1) * parseInt(Math.round(aStepsPerRStep * (i + 1)) - Math.round(aStepsPerRStep * i))});
                    steps.push({radiusStep: 0, azimuthStep: (azimuthStep > 0 ? 1 : -1) * parseInt(Math.round(aStepsPerRStep * (rSteps + 1)) - Math.round(aStepsPerRStep * rSteps))});
                }
            } else steps.push({radiusStep, azimuthStep});
        });

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
const batchSize = steps.length > minimumNumberOfBatchesForLargeDrawing ? Math.floor(steps.length / minimumNumberOfBatchesForLargeDrawing) : 1;
const batchAccumulator = [];
const batches = [];
steps.forEach(step => {
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
    var radius = 0;

    context.fillStyle = "#ffffff";
    context.fillRect(0, 0, width, height);
    context.translate(polarOriginX, polarOriginY);

    context.strokeStyle = "#000000";
    context.lineCap = "round";
    context.lineWidth = marbleSize;

    for (let j = 0; j <= maxBatch; j++) {
        const batch = batches[j];
        batch.forEach(step => {
            if (step.azimuthStep == 0 && step.radiusStep == 0) {
                return;
            }

            if (step.azimuthStep != 0) {
                const azimuthDelta = -1 * azimuthStepSize * step.azimuthStep;
                context.rotate(azimuthDelta);
                context.beginPath();
                context.arc(0, 0, Math.abs(radius), 0, -1 * azimuthDelta, azimuthDelta > 0);
                context.stroke();
                if (debugLines) {
                    console.log('Rotating %f', azimuthDelta);
                }
            }
            if (step.radiusStep != 0) {
                const radiusDelta = radiusStepSize * step.radiusStep;
                context.beginPath();
                context.moveTo(radius, 0);
                context.lineTo(radius + radiusDelta, 0);
                context.stroke();

                if (debugLines) {
                    console.log('Pushing from %f to %f', radius, radius + radiusDelta);
                }
                radius += radiusDelta;
            }
        });
    }

    context.lineWidth = 2;
    context.beginPath();
    context.arc(0, 0, maxRadius, 0, 2 * Math.PI);
    context.moveTo(0, -1 * maxRadius);
    context.lineTo(0, -1 * maxRadius + 8);
    context.stroke();

    return [canvas, context];
}