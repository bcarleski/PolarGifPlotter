const { S3Client, GetObjectCommand } = require("@aws-sdk/client-s3")
const { IoTDataPlaneClient, PublishCommand } = require("@aws-sdk/client-iot-data-plane")
const s3 = new S3Client({})
const iot = new IoTDataPlaneClient();
const bucket = 'dynamic-sand-iot-data'
const drawingsS3Key = 'drawings.json'

async function sendMessage(responseTopic, messageType, obj) {
    obj.type = messageType
    const input = {
        topic: responseTopic,
        payload: JSON.stringify(obj),
        contentType: 'application/json'
    }

    const cmd = new PublishCommand(input)
    console.log('Sending to "' + responseTopic + '" the message ' + JSON.stringify(obj))
    await iot.send(cmd)
}

async function tryGetDrawingLine(drawing, line, includeTotalLines) {
    const key = 'drawings/' + drawing + '.json';
    try {
        const drawingS3Content = await s3.send(new GetObjectCommand({Bucket: bucket, Key: key}))
        const json = JSON.parse(await drawingS3Content.Body.transformToString())

        if (json && json.commands && json.commands.length > line) {
            var drawingLine = {drawing, line, command:json.commands[line]}
            if (includeTotalLines) {
                drawingLine.totalLines = json.commands.length
            }
            return drawingLine
        } else {
            console.log('Insufficient lines for drawing ' + key)
        }
    } catch (error) {
        console.log('Could not retrieve or parse the drawing ' + key + ': ' + error)
    }

    return null;
}

async function provideUpdate(responseTopic, messageType, drawing, line) {
    if (typeof drawing === 'string' && drawing.length > 0 && typeof line === 'number' && line >= 0) {
        const drawingLine = await tryGetDrawingLine(drawing, line);
        if (drawingLine) {
            await sendMessage(responseTopic, messageType, drawingLine)
            return { statusCode: 200, body: '{"success":true}' }
        }
    }

    try {
        const drawingsS3Content = await s3.send(new GetObjectCommand({Bucket: bucket, Key: drawingsS3Key}))
        const drawings = JSON.parse(await drawingsS3Content.Body.transformToString())

        if (drawings && drawings.drawings && drawings.drawings.length > 0) {
            const idx = Math.floor(Math.random() * drawings.drawings.length)
            const entry = drawings.drawings[idx]
            const drawingLine = await tryGetDrawingLine(entry.drawing, 0, true)

            if (drawingLine) {
                await sendMessage(responseTopic, messageType, drawingLine)
                return { statusCode: 200, body: '{"success":true}' }
            }
        }
    } catch (error) {
        console.log('Could not retrieve or parse the drawings: ' + error)
    }

    return { statusCode: 404, body: '{"success":false}' }
}

exports.handler = async (event) => {
    console.log('Received event ' + JSON.stringify(event))

    const result = await provideUpdate(event.responseTopic, event.command, event.drawing, event.line)
    console.log('Sending response ' + JSON.stringify(result));
    return result;
}