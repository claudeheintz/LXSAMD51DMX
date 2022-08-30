/**************************************************************************/
/*!
    @file     DMXInputTest.ino
    @author   Claude Heintz
    @license BSD (see SAMDMX.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2020 by Claude Heintz

    Control brightness of LED on PWM_PIN with DMX address 1
    @section  HISTORY

    v1.00 - First release
*/
/**************************************************************************/

//NOTE for optional TX RX Pins, uncomment see SAMD51DMX.h line 490
#include <LXSAMD51DMX.h>

#define PWM_PIN 2
#define DIRECTION_PIN 7
int got_dmx = 0;

void setup() {
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  SAMD51DMX.setDirectionPin(DIRECTION_PIN);
  SAMD51DMX.setDataReceivedCallback(&gotDMXCallback);
  SAMD51DMX.startInput();
  
  Serial.begin(115200);
  while ( ! Serial ) {}					//serial monitor must be opened for loop to run!
  Serial.println("setup complete");
}


// ***************** input callback function *************

void gotDMXCallback(int slots) {
  got_dmx = slots;
}

/************************************************************************

  The main loop checks to see if dmx input is available (got_dmx>0)
  And then reads the level of dimmer 1 to set PWM level of LED
  
*************************************************************************/

void loop() {
  if ( got_dmx ) {
    analogWrite(PWM_PIN,SAMD51DMX.getSlot(1));
    Serial.println("---");
    Serial.println(SAMDMX.getSlot(1));
    Serial.println(SAMDMX.getSlot(2));
    got_dmx = 0;
    Serial.println(got_dmx);
    Serial.println("___");
  }
}
