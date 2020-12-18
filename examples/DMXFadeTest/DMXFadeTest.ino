/**************************************************************************/
/*!
    @file     DMXFadeTest.ino
    @author   Claude Heintz
    @license  BSD (see SAMD51DMX.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2020 by Claude Heintz

    Simple Fade test of SAMD51 DMX Driver
    @section  HISTORY

    v1.00 - First release
*/
/**************************************************************************/
#include <LXSAMD51DMX.h>


uint8_t level = 0;

void setup() {
  SAMD51DMX.setDirectionPin(3);  // Or, wire pins 2 & 3 of MAX485 to v+ for testing
  SAMD51DMX.startOutput();
}

/************************************************************************

  The main loop fades the levels of addresses 1 and 70 and 512 to full
  
*************************************************************************/

void loop() {
 SAMD51DMX.setSlot(1,level);
 SAMD51DMX.setSlot(70,level);
 SAMD51DMX.setSlot(512,level);
 delay(50);
 level++;
}