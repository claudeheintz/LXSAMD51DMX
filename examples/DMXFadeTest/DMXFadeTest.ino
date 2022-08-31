/**************************************************************************/
/*!
    @file     DMXFadeTest.ino
    @author   Claude Heintz
    @license  BSD (see SAMD51DMX.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2020 by Claude Heintz

    Simple Fade test of SAMD51 DMX Driver
    @section  HISTORY

    v1.00 - First release
    
    
    This is the circuit for a simple unisolated DMX Shield for input:

 Arduino                    SN 75176 A or MAX 481CPA
 pin       3k        1k      _______________
 |   GND---/\/\/\-+-/\/\/\--| 1      Vcc 8 |------ +5v
 V                |         |              |                 DMX Output
  RX >------------+    +----| 2        B 7 |---------------- Pin 2
                       |    |              |
  D3 or +V  >----------+----| 3 DE     A 6 |---------------- Pin 3
                            |              |
  TX >----------------------| 4 DI   Gnd 5 |---+------------ Pin 1
                             _______________   |
                330 ohm                       GND
  D2 |-----------/\/\/\-----[ LED ]------------|


*/
/**************************************************************************/


//NOTE for optional TX RX Pins, uncomment see SAMD51DMX.h line 490
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