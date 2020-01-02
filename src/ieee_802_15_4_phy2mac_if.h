/*
 * ieee_802_15_4_phy2mac_if.h
 *
 *  Created on: Dec 17, 2019
 *      Author: dumitru
 */

#ifndef IEEE_802_15_4_PHY2MAC_IF_H_
#define IEEE_802_15_4_PHY2MAC_IF_H_

/***************************Imports*************************************************/

#include "ieee_802_15_4_mac_cfg.h"

/***************************Public Macro Definitions********************************/


/***************************Public Type Definitions********************************/


/***************************Public Data Definitions********************************/


/***************************Public Function Definitions****************************/

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacDataConfirmation
 *!< @brief                  : Interface to PHY Data request confirmation
 *!<                         :
 *!< Parameters              :
 *!<                   Input : status - result of request
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_ToMacDataConfirmation(IEEE_802_15_4_PHY_Enum_t status);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacFrameIndication
 *!< @brief                  : Callback interface for PHY layer to indicate MAC layer
 *!<                         : of incoming request
 *!<                         :
 *!< Parameters              :
 *!<                   Input : psduLength - length of incoming data
 *!<                         : psdu - payload of PHY layer
 *!<                         : ppduLinkQuality - quality of signal
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_ToMacFrameIndication(uint8_t psduLength, uint8_t* psdu, uint8_t ppduLinkQuality);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacCcaConfirmation
 *!< @brief                  : MAC confirmation sent by PHY to confirm MAC CCA request
 *!<                         :
 *!< Parameters              :
 *!<                   Input :
 *!<                         :
 *!<                         :
 *!<                   Output:
 *!< Return                  :
 *!< Critical section YES/NO :
 */
void IEEE_802_15_4_ToMacCcaConfirmation(IEEE_802_15_4_PHY_Enum_t status);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacEdConfirmation
 *!< @brief                  : MAC confirmation sent by PHY to confirm MAC ED request
 *!<                         :
 *!< Parameters              :
 *!<                   Input :
 *!<                         :
 *!<                         :
 *!<                   Output:
 *!< Return                  :
 *!< Critical section YES/NO :
 */
void IEEE_802_15_4_ToMacEdConfirmation(IEEE_802_15_4_PHY_Enum_t status, uint8_t* EnergyLevel);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacGetConfirmation
 *!< @brief                  : MAC confirmation sent by PHY to confirm MAC GET request
 *!<                         :
 *!< Parameters              :
 *!<                   Input :
 *!<                         :
 *!<                         :
 *!<                   Output:
 *!< Return                  :
 *!< Critical section YES/NO :
 */
void IEEE_802_15_4_ToMacGetConfirmation(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacSetTrxStateConfirmation
 *!< @brief                  : MAC confirmation sent by PHY to confirm MAC SET_TRX_STATE request
 *!<                         :
 *!< Parameters              :
 *!<                   Input :
 *!<                         :
 *!<                         :
 *!<                   Output:
 *!< Return                  :
 *!< Critical section YES/NO :
 */
void IEEE_802_15_4_ToMacSetTrxStateConfirmation(IEEE_802_15_4_PHY_Enum_t status);

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacSetConfirmation
 *!< @brief                  : MAC confirmation sent by PHY to confirm MAC SET request
 *!<                         :
 *!< Parameters              :
 *!<                   Input :
 *!<                         :
 *!<                         :
 *!<                   Output:
 *!< Return                  :
 *!< Critical section YES/NO :
 */
void IEEE_802_15_4_ToMacSetConfirmation(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID);



#endif /* IEEE_802_15_4_PHY2MAC_IF_H_ */
