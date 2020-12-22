/**************************************************************************/
/*!
    @file     WioTerminal_DMX_Controller.ino
    @author   Claude Heintz
    @license  BSD (see SAMD51DMX.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2020 by Claude Heintz

    Simple Output UI for SAMD51 DMX Driver and Seeed Wio Terminal
    Use the Wio Terminal 5 way switch to select and modify address/level

    v1.00 - First release
*/
/**************************************************************************/
#include <TFT_eSPI.h>
#include <LXSAMD51DMX.h>

#define DIRECTION_PIN 2
#define DBG_PIN 3

TFT_eSPI tft;

uint8_t field_set_mode = 1;
int address = 1;
uint8_t level = 0;

char astr[4];
char lstr[4];

void setup() {
  pinMode(DBG_PIN, OUTPUT);
  digitalWrite(DBG_PIN, HIGH);
  SAMD51DMX.setDirectionPin(DIRECTION_PIN);  // Or, wire pins 2 & 3 of MAX485 to v+ for testing
  SAMD51DMX.startOutput();

  // UI Basic Layout
  tft.begin();
  tft.setRotation(3);
  // screen is 320x240
  tft.fillRect(0, 0, 320, 240, TFT_DARKGREY);

  tft.setTextColor(TFT_PINK);
  tft.setTextSize(2);
  tft.drawString("Wio Terminal DMX", 65, 10);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Address", 40, 60);
  tft.drawString("Level", 200, 60);

  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);

  updateStatus();
}

void updateStatus() {
  SAMD51DMX.setSlot(address,level);
  
  // screen is 320x240
  tft.fillRect(24, 90, 281, 60, TFT_DARKGREY);
  tft.setTextSize(10);
  
  if ( field_set_mode ) {
    tft.setTextColor(TFT_GREENYELLOW);
  } else {
    tft.setTextColor(TFT_BLACK);
  }
  itoa(address, astr, 10);
  tft.drawString(astr, 25, 100);


  if ( field_set_mode ) {
    tft.setTextColor(TFT_BLACK);
  } else {
    tft.setTextColor(TFT_GREENYELLOW);
  }
  itoa(level, lstr, 10);
  tft.drawString(lstr, 180, 100);
}

/************************************************************************

  The main loop fades the levels of addresses 1 and 70 and 512 to full
  
*************************************************************************/

void loop() {
  
  if (digitalRead(WIO_5S_UP) == LOW) {
    if ( field_set_mode ) {
      address++;
      if (address > 512) {
        address = 1;
      }
      // if address changed, get current level for that address
      level = SAMD51DMX.getSlot(address);
    } else {
      level++;
    }

    updateStatus();
  }
  
  if (digitalRead(WIO_5S_DOWN) == LOW) {
    if ( field_set_mode ) {
      address--;
      if (address == 0) {
        address = 512;
      }
      level = SAMD51DMX.getSlot(address);
    } else {
      level--;
    }
  
      updateStatus();
  }

  if (digitalRead(WIO_5S_LEFT) == LOW) {
    if ( field_set_mode == 0 ) {
      field_set_mode = 1;
      updateStatus();
    }
  }

  if (digitalRead(WIO_5S_RIGHT) == LOW) {
    if ( field_set_mode ) {
      field_set_mode = 0;
      updateStatus();
    }
  }
   delay(100);
}
