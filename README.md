Device starts up:
    Registers for device shadow get/accepted and get/rejected topics, and Dynamic_Sand_Arduino/toDevice
    Set WaitingForGetShadowResponse flag
    Empty publish to $aws/things/Dynamic_Sand_Arduine/shadow/name/drawing/get
    Receive message from $aws/things/Dynamic_Sand_Arduine/shadow/name/drawing/get/accepted
        Clear WaitingForGetShadowResponse flag
        Update drawing, line, totalLines, and command to those in the message
        If the command is not empty, draw the current command
    Receive message from $aws/things/Dynamic_Sand_Arduine/shadow/name/drawing/get/rejected
        Clear WaitingForGetShadowResponse flag
        Set drawing to empty
    If drawing is empty
        Set WaitingForDrawing flag
        Publish {command:'drawing'} to Dynamic_Sand_Arduino/fromDevice
            Rule initiates Lambda
            Lambda determines next drawing, publishes it to Dynamic_Sand_Arduino/toDevice
                {type:'drawing',drawing:'name',totalLines:23,line:6,command:'L0,100'}
    If drawing is not empty and line < totalLines
        Set WaitingForLine flag
        Publish {command:'line',drawing:'current',line:1} with the current drawing name and next line number to Dynamic_Sand_Arduino/fromDevice
            Rule initiates lambda
            Lambda looks up line in drawing, publishes message to Dynamic_Sand_Arduino/toDevice
                If found, return {type:'line',line:6,command:'L0,100'}
                If not found, return {type:'line',drawing:'name',totalLines:23,line:6,command:'L0,100'} with the name of a new drawing and its line
    Receive message from Dynamic_Sand_Arduino/toDevice
        If (type='drawing' && WaitingForDrawing && has required attributes) || (type='line' && WaitingForLine && has all attributes (drawing/totalLines/line/command))
            Clear WaitingForDrawing flag && WaitingForLine flag
            Set current drawing, total lines, and current line
            Report state
            Execute wipe
            Execute command
                If unknown command, publish an error message with type of 'Unknown Command' and the message set to command
        If type='line' && WaitingForLine && has required attributes (line/command)
            Clear WaitingForLine flag
            Set current line
            Report state
            Execute command
                If unknown command, publish an error message with type of 'Unknown Command' and the message set to command
        If unknown type or flags nots set, publish an error message with type of 'Unknown Message' and the message set to the raw incoming message text
    To publish an error message
        Publish message to Dynamic_Sand_Arduino/fromDevice {command:'error',type:'type',drawing:'current',totalLines:23,line:3,message:'text version of message'}
            Rule initiates CloudWatch Log
            Log triggers alert
            Alert triggers email to benjamin@carleski.com
