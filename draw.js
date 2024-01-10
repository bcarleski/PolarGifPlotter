const { createCanvas } = require("canvas");
const fs = require("fs");
const GIFEncoder = require("gifencoder");

const width = 1200;
const height = 1200;
const polarOriginX = width / 2;
const polarOriginY = height / 2;
const marbleSize = 8;
const pointsCsv = fs.readFileSync('./points.csv');
const points = pointsCsv.toString("utf-8").split('\n').map(line => {
    const parts = line.split(',');
    return {x:parseFloat(parts[0]),y:parseFloat(parts[1]),r:parseFloat(parts[2]),a:parseFloat(parts[3])};
});
const batchSize = points.length > 20 ? Math.floor(points.length / 20) : 1;
const batchAccumulator = [];
const batches = [];
points.forEach(point => {
    batchAccumulator.push(point);
    if (batchAccumulator.length == batchSize) {
        batches.push([...batchAccumulator]);
        batchAccumulator.splice(0, batchAccumulator.length);
    }
});
if (batchAccumulator.length > 0) {
    batches.push([...batchAccumulator]);
}

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

const finalImage = getImage(batches.length - 1)[0];
const imageBuffer = finalImage.toBuffer("image/png");
fs.writeFileSync("./image.png", imageBuffer);
console.log("Write final image to image.png");


function getImage(maxBatch, debugLines) {
    const canvas = createCanvas(width, height);
    const context = canvas.getContext("2d");
    var xPos = polarOriginX;
    var yPos = polarOriginY;

    context.fillStyle = "#ffffff";
    context.fillRect(0, 0, width, height);

    context.strokeStyle = "#000000";
    context.beginPath();
    context.arc(xPos, yPos, Math.min(width, height) / 2, 0, 2 * Math.PI);
    context.stroke();

    context.lineCap = "round";
    context.lineWidth = marbleSize;

    for (let j = 0; j <= maxBatch; j++) {
        const batch = batches[j];
        batch.forEach(point => {
            context.beginPath();
            context.moveTo(xPos, yPos);
    
            xPos = polarOriginX + point.x;
            yPos = polarOriginY - point.y;

            if (debugLines) {
                console.log("Drawing line to %f x %f, for point (%f, %f)", xPos, yPos, point.x, point.y);
            }
    
            context.lineTo(xPos, yPos);
            context.stroke();
        });
    }

    return [canvas, context];
}