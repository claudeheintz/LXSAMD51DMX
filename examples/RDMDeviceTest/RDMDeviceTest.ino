/**************************************************************************/
/*!
    @file     RDMDeviceTest.ino
    @author   Claude Heintz
    @license  BSD (see LXSAMD51DMX LICENSE)
    @copyright 2017-2021 by Claude Heintz

    Example showing LXSAMD51DMX RDM support for devices.
    Control brightness of LED on GPIO14 with DMX address 1 (settable via RDM)
    
    @section  HISTORY
    v1.00 - First release  
*/
/**************************************************************************/

// The general default for the LXSAMD51DMX is to use SERCOM 4 with pins 0 and 1 for DMX RX/TX
// Some SERCOM & Pin setups are defined for specific boards.
// Read the LXSAMD51DMX.h file for other options and uncomment/edit the following line to select them:
//#define use_optional_sercom_macros 4

#include <LXSAMD51DMX.h>
#include <rdm/rdm_utility.h>
#include <rdm/UID.h>
#include <rdm/TOD.h>


int got_dmx = 0;
int got_rdm = 0;
uint8_t discovery_enabled = 1;
uint16_t start_address = 1;
uint16_t input_value = 0;

uint8_t device_label[33];

#define DEFAULT_DEVICE_LABEL  "RDM dev test v1.0"
#define MFG_LABEL             "LXDMX"
#define MODEL_DESCRIPTION     "RDMDeviceTest"

#define DIRECTION_PIN 7
#define LED_PIN       6
#define BUILTIN_LED   13

void setup() {
  Serial.begin(115200);
  while ( ! Serial ) {}
  Serial.println("serial begun");
  
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(DIRECTION_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  //diagnostic pins
  pinMode(12, OUTPUT);
  pinMode(16, INPUT_PULLUP);

  strcpy((char*)device_label, DEFAULT_DEVICE_LABEL);
  
  SAMD51DMX.setDataReceivedCallback(&gotDMXCallback);
  SAMD51DMX.setRDMReceivedCallback(&gotRDMCallback);
  LXSAMD51DMX::THIS_DEVICE_ID.setBytes(0x6C, 0x78, 0x0F, 0x0A, 0x0C, 0x0E);    //change device ID from default
  
  SAMD51DMX.startRDM(DIRECTION_PIN, DMX_TASK_RECEIVE);
}


// ***************** input callback function *************

void gotDMXCallback(int slots) {
  got_dmx = slots;
}

void gotRDMCallback(int len) {
  // rdm start code and checksum are validated before this is called
  got_rdm = len;
}

/************************************************************************

  The main loop checks to see if dmx input is available (got_dmx>0)
  And then reads the level of dimmer 1 to set PWM level of LED connected to pin 14
  
*************************************************************************/

void loop() {
  if ( got_dmx ) {
    input_value = SAMD51DMX.getSlot(start_address);
    //gamma correct
    input_value = (input_value * input_value ) / 255;
    analogWrite(LED_PIN,input_value);
    got_dmx = 0;  //reset
    
  } else if ( got_rdm ) {
    
    uint8_t* rdmdata = SAMD51DMX.receivedRDMData();
    
  uint8_t cmdclass = rdmdata[RDM_IDX_CMD_CLASS];
  uint16_t pid = (rdmdata[RDM_IDX_PID_MSB] << 8 ) | rdmdata[RDM_IDX_PID_LSB];
  

  if ( cmdclass == RDM_DISCOVERY_COMMAND ) {
    Serial.println("RDM_DISCOVERY_COMMAND");
    
    if ( pid == RDM_DISC_UNIQUE_BRANCH ) {
      if ( discovery_enabled ) {
        uint64_t tv = SAMD51DMX.THIS_DEVICE_ID.getValue();
        UID u;
        u.setBytes(&rdmdata[24]);                //lower 
        uint64_t uv = u.getValue();
        if ( tv >= uv ) {
          u.setBytes(&rdmdata[30]);              //upper
          uv = u.getValue();
          if ( tv <= uv ) {
            SAMD51DMX.sendRDMDiscoverBranchResponse();
          }
        }
      }
    } else {  // mute RDM_DISCOVERY_COMMAND PIDs
      UID destination;
      destination.setBytes(&rdmdata[RDM_IDX_DESTINATION_UID]);
      
      if ( pid == RDM_DISC_MUTE ) {
        if ( destination == SAMD51DMX.THIS_DEVICE_ID ) {
          discovery_enabled = 0;
          // send ACK
          UID source;
          source.setBytes(&rdmdata[RDM_IDX_SOURCE_UID]);
          SAMD51DMX.sendMuteAckRDMResponse(RDM_DISC_COMMAND_RESPONSE, source, RDM_DISC_MUTE);
        }
      } else if ( pid == RDM_DISC_UNMUTE ) {
        if ( destination == BROADCAST_ALL_DEVICES_ID ) {
          // just un-mute
          discovery_enabled = 1;
        } else if ( destination == SAMD51DMX.THIS_DEVICE_ID ) {
          discovery_enabled = 1;
          // send ACK
          UID source;
          source.setBytes(&rdmdata[RDM_IDX_SOURCE_UID]);
          SAMD51DMX.sendMuteAckRDMResponse(RDM_DISC_COMMAND_RESPONSE, source, RDM_DISC_UNMUTE);
        }
      }
      
    }
      
  } else if ( cmdclass == RDM_GET_COMMAND ) {
    UID destination;
    destination.setBytes(&rdmdata[RDM_IDX_DESTINATION_UID]);
          
    if ( destination == SAMD51DMX.THIS_DEVICE_ID ) {
       UID source;
       source.setBytes(&rdmdata[RDM_IDX_SOURCE_UID]);
       
       if ( pid == RDM_DEVICE_START_ADDR ) {
          
          uint8_t sa[2];
          sa[0] = start_address >> 8;
          sa[1] = start_address & 0xff;
          SAMD51DMX.sendRDMGetResponse(source, pid, sa, 2);
       } else if ( pid == RDM_DEVICE_MFG_LABEL ) {
          const char * label = MFG_LABEL;
          SAMD51DMX.sendRDMGetResponse(source, pid, (uint8_t*)label, 5);
       } else if ( pid == RDM_DEVICE_MODEL_DESC ) {
          const char * label = MODEL_DESCRIPTION;
          SAMD51DMX.sendRDMGetResponse(source, pid, (uint8_t*)label, 13);
       } else if ( pid == RDM_DEVICE_DEV_LABEL ) {
          SAMD51DMX.sendRDMGetResponse(source, pid, device_label, strlen((const char*)device_label));
       }

       
    }
  } else if ( cmdclass == RDM_SET_COMMAND ) {
      UID destination;
      destination.setBytes(&rdmdata[RDM_IDX_DESTINATION_UID]);
          
      if ( destination == SAMD51DMX.THIS_DEVICE_ID ) {
         UID source;
         source.setBytes(&rdmdata[RDM_IDX_SOURCE_UID]);
         
         if ( pid == RDM_DEVICE_START_ADDR ) {
            uint16_t scratch = (rdmdata[24] << 8) + rdmdata[25];
            if (( scratch > 0 ) && ( scratch < 513 )) {
              start_address = scratch;
            }
            SAMD51DMX.sendAckRDMResponse(RDM_SET_COMMAND_RESPONSE, source, pid);
            
         } else if ( pid == RDM_DEVICE_DEV_LABEL ) {
            uint8_t llen = 0;
            if ( rdmdata[2] > 24 ) {  //label not empty string
              llen = rdmdata[2] - 24;
              if ( llen > 32 ) {      //limit to max 32 characters
                llen = 32;
              }
            }
            for ( uint8_t j=0; j<33; j++) { //copy label, zero the rest of the array
              if ( j < llen ) {
                device_label[j] = rdmdata[24+j];
              } else {
                device_label[j] = 0;
              }
            }   // <-for
            SAMD51DMX.sendAckRDMResponse(RDM_SET_COMMAND_RESPONSE, source, pid);
         }      // <-pid RDM_DEVICE_DEV_LABEL
      }
  }
  got_rdm = 0;
  }               //gotRDM
}
