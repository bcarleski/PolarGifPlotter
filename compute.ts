export interface Point {
    readonly x: number; // X coordinate in a Cartesian system
    readonly y: number; // Y coordinate in a Cartesian system
    readonly r: number; // Radius coordinate in a Polar system
    readonly a: number; // Azimuth coordinate in a Polar system
}

function getCommandPoint(command: string, cur: Point, radiusStepSize: number, azimuthStepSize: number): Point {
    const pos = command.substring(1).split(",");

    // Get the raw positions first
    const rawX = parseInt(pos[0]);
    const rawY = parseInt(pos[1]);
    const rawR = Math.sqrt(rawX * rawX + rawY * rawY);
    let rawA = Math.atan2(rawY, rawX);

    const slope = (rawY - cur.y) / (rawX - cur.x);
    const yIntercept = rawY - slope * rawX;
    const xIntercept = -1 * yIntercept / slope;

    // Ensure we sweep the correct direction given where the X and Y intercepts of the line are in the correct direction
    if (   (xIntercept < 0 && yIntercept < 0 && cur.x > rawX)
        || (xIntercept < 0 && yIntercept > 0 && cur.x < rawX)
        || (xIntercept > 0 && yIntercept < 0 && cur.x > rawX)
        || (xIntercept > 0 && yIntercept > 0 && cur.x < rawX)) {
        while (rawA > cur.a) {
            rawA -= Math.PI * 2;
        }
    }
    // If we are moving left to right below the Y axis, or right to left above it, the azimuth should be increasing
    else if ((xIntercept < 0 && yIntercept < 0 && cur.x < rawX)
        ||   (xIntercept < 0 && yIntercept > 0 && cur.x > rawX)
        ||   (xIntercept > 0 && yIntercept < 0 && cur.x < rawX)
        ||   (xIntercept > 0 && yIntercept > 0 && cur.x > rawX)) {
        while (rawA < cur.a) {
            rawA += Math.PI * 2;
        }
    }

    const pt = getPoint(rawR, rawA, radiusStepSize, azimuthStepSize);

    console.log('Got command point, raw (%f x %f) / (%f x %f), rounded (%f x %f) / (%f x %f)',
        rawX, rawY, rawR, rawA, pt.x, pt.y, pt.r, pt.a);

    return pt;
}

function getPoint(radius: number, azimuth: number, radiusStepSize?: number, azimuthStepSize?: number): Point {
    // Then find the closest point that is a whole multiple of the step sizes
    const r = radiusStepSize ? Math.round(radius / radiusStepSize) * radiusStepSize : radius;
    const a = azimuthStepSize ? Math.round(azimuth / azimuthStepSize) * azimuthStepSize : azimuth;
    const x = radius * Math.cos(azimuth);
    const y = radius * Math.sin(azimuth);

    return {x,y,r,a};
}

function addLineSegments(dest: Point, points: Point[], radiusStepSize: number, azimuthStepSize: number) {
    const cur = points[points.length - 1];
    const startA = cur.r === 0 ? dest.a : cur.a;
    const radiusSteps = Math.round(Math.abs(dest.r - cur.r) / radiusStepSize);
    const azimuthSteps = Math.round(Math.abs(dest.a - startA) / azimuthStepSize);

    console.log('Adding line segment from (%f x %f) / (%f x %f) to (%f x %f) / (%f x %f) in rSteps=%f and aSteps=%f',
        cur.x, cur.y, cur.r, startA, dest.x, dest.y, dest.r, dest.a, radiusSteps, azimuthSteps);

    if (radiusSteps === 0 && azimuthSteps === 0) {
        return;
    }

    const moreRadiusSteps = radiusSteps > azimuthSteps;
    const majorSteps = moreRadiusSteps ? radiusSteps : azimuthSteps;
    const radiusStepOffset = dest.r > cur.r ? radiusStepSize : (-1 * radiusStepSize);
    const azimuthStepOffset = dest.a > startA ? azimuthStepSize : (-1 * azimuthStepSize);
    let minorStep = 0;

    console.log('    MoreRadiusSteps=%s, MajorSteps=%d, RadiusStepOffset=%f, AzimuthStepOffset=%f',
        moreRadiusSteps, majorSteps, radiusStepOffset, azimuthStepOffset);

    for (let majorStep = 1; majorStep <= majorSteps; majorStep++) {
        const radius = cur.r + (moreRadiusSteps ? majorStep : minorStep) * radiusStepOffset;
        const azimuth = startA + (moreRadiusSteps ? minorStep : majorStep) * azimuthStepOffset;
        const curPoint = getPoint(radius, azimuth);
        let upPoint: Point;
        let downPoint: Point;

        if (moreRadiusSteps) {
            upPoint = getPoint(radius, azimuth + azimuthStepOffset);
            downPoint = getPoint(radius, azimuth - azimuthStepOffset);
        } else {
            upPoint = getPoint(radius + radiusStepOffset, azimuth);
            downPoint = getPoint(radius - radiusStepOffset, azimuth);
        }

        const distanceToCurrent = findDistanceToLine(curPoint, cur, dest);
        const distanceToUp = findDistanceToLine(upPoint, cur, dest);
        const distanceToDown = findDistanceToLine(downPoint, cur, dest);

        if (distanceToUp < distanceToDown && distanceToUp < distanceToCurrent) {
            minorStep++;
            points.push(upPoint);
        } else if (distanceToDown < distanceToUp && distanceToDown < distanceToCurrent) {
            minorStep--;
            points.push(downPoint);
        } else {
            points.push(curPoint);
        }
    }

    const pt = points[points.length - 1];
    console.log('Made it to %f,%f,%f,%f', pt.x, pt.y, pt.r, pt.a);
}

function findDistanceToLine(test: Point, linePoint1: Point, linePoint2: Point): number {
    // From https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
    const x0 = test.x;
    const x1 = linePoint1.x;
    const x2 = linePoint2.x;
    const xDelta = x2 - x1;
    const y0 = test.y;
    const y1 = linePoint1.y;
    const y2 = linePoint2.y;
    const yDelta = y2 - y1;

    const numerator = Math.abs(xDelta * (y1 - y0) - (x1 - x0) * yDelta);
    const denominator = Math.sqrt(xDelta * xDelta + yDelta * yDelta);

    return numerator / denominator;
}

export class PointComputer {
    public readonly points: Point[];

    constructor(radiusStepSize: number, azimuthStepSize: number, commands: string[]) {
        const points: Point[] = [{x:0,y:0,r:0,a:0}];
    
        for (let i = 0; i < commands.length; i++) {
            const cur = points[points.length - 1];
            const command = commands[i];
            const type = command.charAt(0);
    
            switch (type) {
                case "L":
                    const dest = getCommandPoint(command, cur, radiusStepSize, azimuthStepSize);
                    addLineSegments(dest, points, radiusStepSize, azimuthStepSize);
                    break;
                default:
                    throw "Unknown line type " + type;
            }
        }

        this.points = points;
    }
}