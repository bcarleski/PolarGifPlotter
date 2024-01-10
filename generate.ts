import { writeFile } from "fs";
import { PointComputer } from "./compute";

const radiusStepSize = 2.0;
const azimuthStepSize = Math.PI / 360.0;
const points = new PointComputer(radiusStepSize, azimuthStepSize, [
    "L-200,200",
    "L200,200",
    "L200,-200",
    "L-200,-200",
    "L-200,200"
]).points;

const content = points.map(point => [point.x, point.y, point.r, point.a].join(',')).join('\n');
const buffer = Buffer.from(content, "utf-8");

writeFile("./points.csv", buffer, err => console.log(err));