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
#include <rdm/rdm_utility.h>
#include <rdm/UID.h>
#include <rdm/TOD.h>


uint8_t testLevel = 0;
uint8_t loopDivider = 0;
uint8_t identifyFlag = 1;
uint8_t tableChangedFlag = 0;

TOD tableOfDevices;
TOD discoveryTree;

UID lower(0,0,0,0,0,0);
UID upper(0,0,0,0,0,0);
UID mid(0,0,0,0,0,0);
UID found(0,0,0,0,0,0);

#define DIRECTION_PIN 2
#define DISC_STATE_SEARCH 0
#define DISC_STATE_TBL_CK 1
uint8_t discovery_state = DISC_STATE_TBL_CK;
uint8_t discovery_tbl_ck_index = 0;



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
  testRDMDiscovery();
  
  SAMD51DMX.setSlot(101,testLevel);
  SAMD51DMX.setSlot(103,255);
  loopDivider++;
  if ( loopDivider == 4 ) {
    testLevel++;
    loopDivider = 0;
  }
  if ( testLevel == 1 ) {
    delay(500);
    identifyFlag = 1;
  }
}

/*************************************************** 
 *     RDM discovery functions
 *     
 *     Sends RDM discovery packets/messages and builds a table of devices.
 *     
 *     RDM discovery packets ask the DMX chain for responses to a range of UIDs
 *     The range is divided each time a response is received.
 *     Repeated discovery/divide cycles close in on individual device UIDs
 *     
 *     When an UID is isolated, it is muted so it does not respond to additional discovery packets.
 *     When a device accepts a mute message, it's existence on the chain is confirmed
 *     and it's UID is added to the table of devices
 *     
 ***************************************************/

void testRDMDiscovery() {
  if ( discovery_state ) {
    // check the table of devices
    discovery_tbl_ck_index = checkTable(discovery_tbl_ck_index);
    if ( discovery_tbl_ck_index == 0 ) {
      // done with table check
      discovery_state = DISC_STATE_SEARCH;
      pushInitialBranch();

      if ( identifyFlag ) {   //once per cycle identify each device
        identifyEach();       //this is just to demonstrate GET device address
        identifyFlag = 0;     //and SET identify device
      }
      
      if ( tableChangedFlag ) {   //if the table has changed...
        tableChangedFlag = 0;

        // if this were an Art-Net application, you would send an 
        // ArtTOD packet here, because the device table has changed.
        // for this test, we just print the list of devices
         Serial.println("_______________ Table Of Devices _______________");
         tableOfDevices.printTOD();
      }
    }
  } else {    // search for devices in range popped from discoveryTree
    if ( checkNextRange() == 0 ) {
      // done with search
      discovery_tbl_ck_index = 0;
      discovery_state = DISC_STATE_TBL_CK;
    }
  }
}

void checkDeviceFound(UID found) {
  Serial.print("Check device: ");
  Serial.println(found);
  if ( testMute(found) ) {        // If the device UID replies, it is muted, so
    Serial.println("found one!"); // it won't respond to other discovery messages.
    tableOfDevices.add(found);    // This confirms the device on the DMX chain:
    tableChangedFlag = 1;         // Add it to the table of devices.
  }
}

uint8_t testMute(UID u) {
   // try three times to get response when sending a mute message
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return 1;
   }
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return 1;
   }
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return 1;
   }
   return 0;
}

uint8_t checkTable(uint8_t ck_index) {
  if ( ck_index == 0 ) {
    SAMD51DMX.sendRDMDiscoveryMute(BROADCAST_ALL_DEVICES_ID, RDM_DISC_UNMUTE);
  }

  if ( tableOfDevices.getUIDAt(ck_index, &found) )  {
    if ( testMute(found) ) {
      // device confirmed
      return ck_index += 6;
    }
    
    // device not found
    tableOfDevices.removeUIDAt(ck_index);
    tableChangedFlag = 1;
    return ck_index;
  }
  // index invalid
  return 0;
}

void identifyEach() { // Send an identify command to each device in the table
  int i = 0;
  uint8_t notDone = 1;
  while ( notDone ) {
    i = tableOfDevices.getNextUID(i, &found);
    if ( i < 0 ) {
      notDone = 0;
    } else {
      //uint16_t data;  //for DMX address and identify device on/off
      uint8_t data[2];
      if ( SAMD51DMX.sendRDMGetCommand(found, RDM_DEVICE_START_ADDR, data, 2) ) {
        uint16_t addr = (data[0] << 8) | data[1];

        if ( addr == 0x0F ) {
          data[0] = 0x00;
          data[1] = 0x01;
          SAMD51DMX.sendRDMSetCommand(found, RDM_DEVICE_START_ADDR, (uint8_t*)data, 2);
        }
  
        data[0] = 0x01;
        SAMD51DMX.sendRDMSetCommand(found, RDM_IDENTIFY_DEVICE, (uint8_t*)data, 1);
        delay(2000);
        data[0] = 0x00;
        SAMD51DMX.sendRDMSetCommand(found, RDM_IDENTIFY_DEVICE, (uint8_t*)data, 1);
      }
    }
  }
}

//called when range responded, so divide into sub ranges push them on stack to be further checked
void pushActiveBranch(UID lower, UID upper) {
  if ( mid.becomeMidpoint(lower, upper) ) {
    discoveryTree.push(lower);
    discoveryTree.push(mid);
    discoveryTree.push(mid);
    discoveryTree.push(upper);
  } else {
    // No midpoint possible:  lower and upper are equal or a 1 apart
    checkDeviceFound(lower);
    checkDeviceFound(upper);
  }
}

void pushInitialBranch() {  // initial range of devices for identify
  lower.setBytes(0);
  upper.setBytes(BROADCAST_ALL_DEVICES_ID);
  discoveryTree.push(lower);
  discoveryTree.push(upper);

  //ETC devices seem to only respond with wildcard or exact manufacturer ID
  lower.setBytes(0x657400000000);
  upper.setBytes(0x6574FFFFFFFF);
  discoveryTree.push(lower);
  discoveryTree.push(upper);
}

uint8_t checkNextRange() {    
  if ( discoveryTree.pop(&upper) ) {    // pop UIDs off the list to be checked
    if ( discoveryTree.pop(&lower) ) {
      if ( lower == upper ) {
        checkDeviceFound(lower);
      } else {        //not leaf so, check range lower->upper
        uint8_t result = SAMD51DMX.sendRDMDiscoveryPacket(lower, upper, &found);
        if ( result ) {
          //this range responded, so divide into sub ranges push them on stack to be further checked
          pushActiveBranch(lower, upper);
           
        } else if ( SAMD51DMX.sendRDMDiscoveryPacket(lower, upper, &found) ) {
            pushActiveBranch(lower, upper); //if discovery fails, try a second time
        }
      }         // end check range
      return 1; // UID ranges may be remaining to test
    }           // end valid pop
  }             // end valid pop  
  return 0;     // none left to pop
}
