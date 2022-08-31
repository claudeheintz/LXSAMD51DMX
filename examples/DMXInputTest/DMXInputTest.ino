/**************************************************************************/
/*!
    @file     DMXInputTest.ino
    @author   Claude Heintz
    @license BSD (see SAMDMX.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2020 by Claude Heintz

    Control brightness of LED on PWM_PIN with DMX address 1
    @section  HISTORY

    v1.00 - First release
    
    
    This is the circuit for a simple unisolated DMX Shield for input:

 Arduino                    SN 75176 A or MAX 481CPA
 pin       3k        1k      _______________
 |   GND---/\/\/\-+-/\/\/\--| 1      Vcc 8 |------ +5v
 V                |         |              |                 DMX Output
  RX >------------+    +----| 2        B 7 |---------------- Pin 2
                       |    |              |
  D7 or GND >----------+----| 3 DE     A 6 |---------------- Pin 3
                            |              |
  TX >----------------------| 4 DI   Gnd 5 |---+------------ Pin 1
                             _______________   |
                330 ohm                       GND
  D2 |-----------/\/\/\-----[ LED ]------------|


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

  SAMD51DMX.setDirectionPin(DIRECTION_PIN);  // Or, wire pins 2 & 3 of MAX485 to GND for testing
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
    Serial.println(SAMD51DMX.getSlot(1));
    Serial.println(SAMD51DMX.getSlot(2));
    got_dmx = 0;
    Serial.println(got_dmx);
    Serial.println("___");
  }
}
