/**************************************************************************/
/*!
    @file     LXRDMDiscovery.cpp
    @author   Claude Heintz
    @license  BSD (see LXDMXWiFi.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2021 by Claude Heintz All Rights Reserved
*/
/**************************************************************************/

#include <Arduino.h>
#include "LXRDMDiscovery.h"

LXRDMDiscovery RDM_Discovery;

void LXRDMDiscovery::updateRDMDiscovery() {
  if ( discovery_state ) {
    // check the table of devices
    discovery_tbl_ck_index = checkTable(discovery_tbl_ck_index);
    if ( discovery_tbl_ck_index == RDM_DONE ) {
      // done with table check
      discovery_state = RDM_DISC_STATE_SEARCH;
      pushInitialBranch();

      if ( identifyFlag ) {   //once per cycle identify each device
        identifyEach();       //this is just to demonstrate GET device address
        identifyFlag = RDM_DONT_IDENTIFY;     //and SET identify device
      }
      
      if ( tableChangedFlag ) {   //if the table has changed...
        tableChangedFlag = RDM_TABLE_UNCHANGED;

        // if this were an Art-Net application, you would send an 
        // ArtTOD packet here, because the device table has changed.
        // for this test, we just print the list of devices
         Serial.println("_______________ Table Of Devices _______________");
         tableOfDevices.printTOD();
      }
    }
  } else {    // search for devices in range popped from discoveryTree
    if ( checkNextRange() == RDM_DONE ) {
      // done with search
      discovery_tbl_ck_index = 0;
      discovery_state = RDM_DISC_STATE_TBL_CK;
    }
  }
}

void LXRDMDiscovery::setIdentifyFlag(uint8_t f) {
  identifyFlag = f;
}

void LXRDMDiscovery::checkDeviceFound(UID* found) {
  Serial.print("Check device: ");
  Serial.println(*found);
  if ( testMute(found) ) {
    Serial.println("found one!");
    tableOfDevices.add(*found);
    tableChangedFlag = RDM_TABLE_CHANGED;
  }
}


uint8_t LXRDMDiscovery::testMute(UID* u) {
   // try three times to get response when sending a mute message
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return RDM_MUTE_ACCEPT;
   }
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return RDM_MUTE_ACCEPT;
   }
   if ( SAMD51DMX.sendRDMDiscoveryMute(u, RDM_DISC_MUTE) ) {
     return RDM_MUTE_ACCEPT;
   }
   return RDM_MUTE_NOREPLY;
}

uint8_t LXRDMDiscovery::checkTable(uint8_t ck_index) {
  if ( ck_index == 0 ) {  // init table check
    SAMD51DMX.sendRDMDiscoveryMute((UID*)&BROADCAST_ALL_DEVICES_ID, RDM_DISC_UNMUTE);
  }
  if ( tableOfDevices.getUIDAt(ck_index, &deviceID) )  {
    if ( testMute(&deviceID) ) {
      // device confirmed
      return ck_index += 6;
    }
    
    // device not found
    tableOfDevices.removeUIDAt(ck_index);
    tableChangedFlag = 1;
    return ck_index;
  }
  // index invalid, done with check
  return RDM_DONE;
}

/*
 *  Send an identify message to each device in the tableOfDevices
 */

void LXRDMDiscovery::identifyEach() { // Send an identify command to each device in the table
  int i = 0;
  uint8_t notDone = RDM_NOT_DONE;
  while ( notDone ) {
    i = tableOfDevices.getNextUID(i, &deviceID);
    if ( i < 0 ) {
      notDone = RDM_DONE;
    } else {
      //uint16_t data;  //for DMX address and identify device on/off
      uint8_t data[2];
      if ( SAMD51DMX.sendRDMGetCommand(&deviceID, RDM_DEVICE_START_ADDR, data, 2) ) {
        uint16_t addr = (data[0] << 8) | data[1];

        if ( addr == 0x0F ) {
          data[0] = 0x00;
          data[1] = 0x01;
          SAMD51DMX.sendRDMSetCommand(&deviceID, RDM_DEVICE_START_ADDR, (uint8_t*)data, 2);
        }
  
        data[0] = 0x01;
        SAMD51DMX.sendRDMSetCommand(&deviceID, RDM_IDENTIFY_DEVICE, (uint8_t*)data, 1);
        delay(2000);
        data[0] = 0x00;
        SAMD51DMX.sendRDMSetCommand(&deviceID, RDM_IDENTIFY_DEVICE, (uint8_t*)data, 1);
      }
    }
  }
}

void LXRDMDiscovery::pushActiveBranch() {
  if ( mid.becomeMidpoint(lower, upper) ) {
    discoveryTree.push(lower);
    discoveryTree.push(mid);
    discoveryTree.push(mid);
    discoveryTree.push(upper);
  } else {
    // No midpoint possible:  lower and upper are equal or a 1 apart
    checkDeviceFound(&lower);
    checkDeviceFound(&upper);
  }
}

void LXRDMDiscovery::pushInitialBranch() {  // initial range of devices for identify
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

uint8_t LXRDMDiscovery::checkNextRange() {    
  if ( discoveryTree.pop(&upper) ) {    // pop UIDs off the list to be checked
    if ( discoveryTree.pop(&lower) ) {
      if ( lower == upper ) {
        checkDeviceFound(&lower);
      } else {        // not leaf so, check range lower->upper
        UID found;    // UID is filled by sendRDMDiscoveryPacket on a response but this not used...
        uint8_t result = SAMD51DMX.sendRDMDiscoveryPacket(&lower, &upper, &found);
        if ( result ) {
          //this range responded, so divide into sub ranges push them on stack to be further checked
          pushActiveBranch();
           
        } else if ( SAMD51DMX.sendRDMDiscoveryPacket(&lower, &upper, &found) ) {
            pushActiveBranch(); //if discovery fails, try a second time
        }
      }                    // end check range
      return RDM_NOT_DONE; // UID ranges may be remaining to test
    }                      // end valid pop
  }                        // end valid pop  
  return RDM_DONE;         // none left to pop
}
