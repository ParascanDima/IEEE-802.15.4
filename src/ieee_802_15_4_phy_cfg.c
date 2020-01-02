/*
 * ieee_802_15_4_phy_cfg.c
 *
 *  Created on: Dec 17, 2019
 *      Author: dumitru
 */
/**************Imports***********************************/

#include "ieee_802_15_4_phy_cfg.h"
#include "ieee_802_15_4_phy2mac_if.h"

/**************Private Macro Definitions*****************/


/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/


/**************Public Variable Definitions***************/

/*!<
 *!< @brief PHY Pointers to Chip specific config and interfaces to upper layers
 *!< */
const voidFuncPtr DataRequestChipSpecificCallback = NULL;
const PD_Data_Confirm_t DataConfirmationCallback = IEEE_802_15_4_ToMacDataConfirmation;
const PD_Data_Indication_t DataIndicationCallback = IEEE_802_15_4_ToMacFrameIndication;

const PLME_CCA_Request_t CcaRequestChipSpecificCallback = NULL;
const PLME_CCA_Confirm_t CcaConfirmCallback = IEEE_802_15_4_ToMacCcaConfirmation;

const PLME_ED_Request_t EdRequestChipSpecificCallback = NULL;
const PLME_ED_Confirm_t EdConfirmCallback = IEEE_802_15_4_ToMacEdConfirmation;

const PLME_GET_Request_t GetRequestChipSpecificCallback = NULL;
const PLME_GET_Confirm_t GetConfirmCallback = IEEE_802_15_4_ToMacGetConfirmation;

const PLME_SET_TRX_STATE_Request_t SetTrxStateRequestChipSpecificCallback = NULL;
const PLME_SET_TRX_STATE_Confirm_t SetTrxStateConfirmCallback = IEEE_802_15_4_ToMacSetTrxStateConfirmation;

const voidu8FuncPtr SetPhyChannelCallback = NULL;
const voidu32FuncPtr SetSupportedChannelsCallback = NULL;
const voidu8FuncPtr SetTransmitPowerCallback = NULL;
const voidu8FuncPtr SetCCAModeCallback = NULL;

const PLME_SET_Request_t SetRequestChipSpecificCallback = NULL;
const PLME_SET_Confirm_t SetConfirmCallback = IEEE_802_15_4_ToMacSetConfirmation;


const IEEE_802_15_4_PHY_Enum_t (*GetTranceiverState)(void) = NULL;


/**************Private Function Definitions**************/


/**************Public Function Definitions***************/
