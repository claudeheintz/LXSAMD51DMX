/**************************************************************************/
/*!
    @file     LXRDMDiscovery.h
    @author   Claude Heintz
    @license  BSD (see LXDMXWiFi.h or http://lx.claudeheintzdesign.com/opensource.html)
    @copyright 2021 by Claude Heintz All Rights Reserved
    
    
*/
/**************************************************************************/

#include <inttypes.h>
#include <LXSAMD51DMX.h>
#include <rdm/rdm_utility.h>
#include <rdm/UID.h>
#include <rdm/TOD.h>


// Constants
#define RDM_DISC_STATE_SEARCH 0
#define RDM_DISC_STATE_TBL_CK 1

#define RDM_TABLE_UNCHANGED  0
#define RDM_TABLE_CHANGED    1

#define RDM_MUTE_NOREPLY  0
#define RDM_MUTE_ACCEPT   1

#define RDM_DONT_IDENTIFY 0
#define RDM_IDENTIFY_ALL  1

#define RDM_DONE      0
#define RDM_NOT_DONE  1

/*************************************************** 
 *     RDM discovery functions
 *     
 *     Sends RDM discovery packets/messages and builds a table of devices.
 *     
 *     RDM discovery packets ask the DMX chain for responses to a range of UIDs
 *     The range is divided each time a response is received.
 *     Repeated discovery/divide cycles close in on individual device UIDs
 *     
 *     When an UID is isolated, it is muted so it does not respond
 *     to additional discovery packets.
 *     When a device accepts a mute message, it's existence on the chain is confirmed
 *     and it's UID is added to the table of devices
 *     
 ***************************************************/

/*!   
@class LXRDMDiscovery
@abstract
   Class encapsulates RDM discovery process.
*/

class LXRDMDiscovery {

  public:
  
   /*
  	*  __updateRDMDiscovery__
	  *  updateRDMDiscovery is the main entry into RDM discovery
	  *  should be called on the main loop to execute the next step
	  *  in the discovery process
	  */
	void updateRDMDiscovery();


  /*
   * Set the flag for sending identify messages on the next updateRDMDiscovery cycle.
   */
  void setIdentifyFlag(uint8_t f);
	 

  private:
  
	/*
	 *  __checkDeviceFound__
	 *	Try to mute the device with UID
	 *  This accomplishes two things:
	 *     The device won't respond to later discovery messages,
	 *     allowing other devices a chance to reply.
	 *     Also, by acknowledging the mute message.
	 *     the device's existence on the DMX chain is confirmed.
	 */
	void checkDeviceFound(UID* found);
	
	/*
	 *  __testMute__
	 *  Try three times to mute the device.
	 *  Return RDM_MUTE_ACCEPT if mute is acknowledged,
	 *  or RDM_MUTE_NOREPLY if not.
	 */
	uint8_t testMute(UID* u);
	
	/*
	 *  __checkTable__
	 *  Confirm all discovered devices exist on DMX chain.
	 *  Unmute all and send individual mutes to
	 *  each device in the tableOfDevices.
	 *  If the mute is acknowledged, keep the device in the table.
	 *  If not, remove the UID from the table.
	 */
	uint8_t checkTable(uint8_t ck_index);
	
	/*
	 *  __identifyEach__
	 *  Send an identify message to each device in the tableOfDevices
	 */
	void identifyEach();
	
	/*
	 *  __pushActiveBranch__
	 *  Called when range responded,
	 *  so divide into sub ranges push them on stack to be further checked
	 *  If no division is possible,
	 *  Check remaining UIDs for a response to a mute, and add to table if accepted
	 */
	void pushActiveBranch();
	
	/*
	 *  __pushInitialBranch__
	 *  Populate the discovery tree with range of UIDs to be checked
	 *  to discover devices.
	 */
	void pushInitialBranch();

	/*
	 *  __checkNextRange__
	 *  Check the next range of UIDs in the discoveryTree for any response
	 *  to a discovery message.
	 */
	uint8_t checkNextRange();
	
	/*  member variables */
   
  TOD tableOfDevices;
	TOD discoveryTree;

  // these are created once and re-used to save allocating and deallocating...
  UID deviceID{};
	UID lower{};
	UID upper{};
	UID   mid{};

  uint8_t identifyFlag = RDM_DONT_IDENTIFY;
  uint8_t tableChangedFlag = RDM_TABLE_UNCHANGED;
	uint8_t discovery_state = RDM_DISC_STATE_TBL_CK;
	uint8_t discovery_tbl_ck_index = 0;
  	 
};

extern LXRDMDiscovery RDM_Discovery;
