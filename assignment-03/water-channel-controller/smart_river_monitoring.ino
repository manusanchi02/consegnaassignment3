/**
 * @author lorenzo.annibalini@studio.unibo.it 	Annibalini Lorenzo
 * @author lorenzo.bacchini4@studio.unibo.it 	Bacchini Lorenzo
 * @author emanuele.sanchi@studio.unibo.it 		Sanchi Emanuele
 */

#include "Button.h"
#include "Gate.h"
#include "LcdMonitor.h"
#include "Led.h"
#include <Arduino.h>
#include <EnableInterrupt.h>
void setUnsetManualMode();

#define LEDPIN1 10
#define LEDPIN2 11
#define LCDROWS 4
#define LCDCOLS 20
#define GATEPIN 13
#define GATEOPEN 180
#define GATECLOSE 0
#define BUTTONPIN 2

#define normalLevel 45
#define alarm_to_highLevel 90

Button *button;
Gate *gate;
LcdMonitor *lcd;
Led *ledGreen;
Led *ledRed;

int manualLevel = 90;
int level;
int gatePos = 0;
String data;
String state;
String previousRead = "blank";

volatile enum controllerState {
    automatic,
    manual
} controllerState;

/**
 * Setup function, called once when the program starts.
 */
void setup()
{
    // calibrate sensors.
    button = new Button(BUTTONPIN);
    gate = new Gate(GATEPIN, GATEOPEN, GATECLOSE);
    lcd = new LcdMonitor(LCDROWS, LCDCOLS);
    ledGreen = new Led(LEDPIN1);
    ledRed = new Led(LEDPIN2);
    lcd->setAndPrint("Calibrating sensors", 0, 0);
    gate->setOpen();
    delay(1000);
    gate->setClose();
    delay(4000);
    lcd->clean();
    enableInterrupt(BUTTONPIN, setUnsetManualMode, CHANGE);
    Serial.begin(9600);
    controllerState = automatic;
    // first lcd print
    lcd->clean();
    lcd->setAndPrint("Valve opening level:", 0, 0);
    lcd->setAndPrint(String(level) + "%", 0, 1);
    lcd->setAndPrint("current modality:", 0, 2);
}

/**
 * Function called when button is pressed to enable and disable manual mode 
*/
void setUnsetManualMode()
{
    if (button->isPressed())
    {
        if (controllerState == manual)
        {
            controllerState = automatic;
        }
        else
        {
            controllerState = manual;
        }
    }
}

/**
 * Function to read from serial in string format
*/
String readStringFromSerial()
{
    String inputString = "";
    char incomingByte;

    while (Serial.available() > 0)
    {
        // read next character
        Serial.readBytes(&incomingByte, 1);
        //add character to inputString
        inputString += incomingByte;
    }
    return inputString;
}

/**
 * Function called to make the fsm take a step forward
 */
void step()
{

    // read data from River Monitoring Service (state and level)
    data = readStringFromSerial();
    if (data.length() > 0)
    {
        level = data.toInt();
    }
    if (data != previousRead && level >= 0 && level <= 100)
    {
        gatePos = map(level, 0, 100, 0, 180);
        if(gatePos > 170){
            gate->setOpen();
        }
        else if(gatePos < 10){
            gate->setClose();
        } else {
            gate->setPos(gatePos);
        }
        lcd->clean();
        lcd->setAndPrint("Valve opening level:", 0, 0);
        lcd->setAndPrint(String(level) + "%", 0, 1);
        lcd->setAndPrint("current modality:", 0, 2);
        previousRead = data;

    }
    lcd->setAndPrint(((controllerState == manual) ? "manual" : "automatic"), 0, 3);
}

/**
 * Loop function called continuously to make
 * the fsm take a step forward.
 */
void loop()
{
    step();
    delay(500);
}