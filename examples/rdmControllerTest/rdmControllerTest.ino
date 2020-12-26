/**************************************************************************/
/*!
    @file     rdmControllerTest.ino
    @author   Claude Heintz
    @license  BSD (see LXSAMD51DMX.h LICENSE)
    @copyright 2017 by Claude Heintz

        Test of LXSAMD51DMX RDM functions
        Changes output level of some DMX Addresses while building RDM
        table of devices.  Turns identify on and off for found RDM devices.

    @section  HISTORY

    v1.00 - First release
*/
/**************************************************************************/

#include <LXSAMD51DMX.h>
#include "LXRDMDiscovery.h"


uint8_t testLevel = 0;
uint8_t loopDivider = 0;


#define DIRECTION_PIN 2

#define INTENSITY_ADDRESS 101
#define COLOR_ADDRESS 103
#define COLOR_LEVEL 255

/************************************************************************
	setup
*************************************************************************/
void setup() {
  Serial.begin(115200);
  while( ! Serial ) {}
  Serial.print("setup... ");

  // debug pins
  //pinMode(6, OUTPUT);
  //digitalWrite(6, LOW);
  //pinMode(8, OUTPUT);
  //digitalWrite(8, LOW);
  
  SAMD51DMX.startRDM(DIRECTION_PIN, RDM_DIRECTION_OUTPUT);
  Serial.println("setup complete");
}


/************************************************************************

  The main loop executes the next step of the RDM discovery process
  by calling testRDMDiscovery().

  Discovered devices have their IDs printed to the Serial output.
  They also have their identify function triggered (generally flashing themselves).
  
  It also increments an output level every 4 times through the loop.
  
     (This example shows a constant intensity address for an LED fixture at full
      and a changing level for one of the colors)
  
*************************************************************************/

void loop() {
  delay(2);
  RDM_Discovery.updateRDMDiscovery();
  
  SAMD51DMX.setSlot(INTENSITY_ADDRESS, testLevel);
  SAMD51DMX.setSlot(COLOR_ADDRESS, COLOR_LEVEL);
  loopDivider++;
  if ( loopDivider == 4 ) {
    testLevel++;
    loopDivider = 0;
  }
  if ( testLevel == 1 ) {
    delay(500);
    // send identify cmd to all devices in deviceTable, once every cycle of testLevel
    RDM_Discovery.setIdentifyFlag(RDM_IDENTIFY_ALL);  
  }
}
