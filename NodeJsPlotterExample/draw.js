const { createCanvas } = require("canvas");
const fs = require("fs");
const GIFEncoder = require("gifencoder");

const CSV_FILE_PATH = './serial.log';
const width = 1200;
const height = 1200;
const polarOriginX = width / 2;
const polarOriginY = height / 2;
const enableGif = false;
const enablePng = true;
const marbleSize = 8;
const radiusStepSize = 4;
const azimuthStepSize = -1 * Math.PI / 180.0;
const stepsCsv = fs.readFileSync(CSV_FILE_PATH);
const steps = stepsCsv.toString("utf-8")
                      .split('\n')
                      .filter(line => line.length > 0 && line.indexOf(',') > 0 && line.indexOf(' ') === -1)
                      .map(line => {
                          const parts = line.split(',');
                          return {radiusStep:parseInt(parts[0]),azimuthStep:parseInt(parts[1])};
                      });
const batchSize = steps.length > 20 ? Math.floor(steps.length / 20) : 1;
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
    const finalImage = getImage(batches.length - 1, false)[0];
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
                const azimuthDelta = azimuthStepSize * step.azimuthStep;
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