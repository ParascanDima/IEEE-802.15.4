/*
 * ieee_802_15_4_phy_cfg.h
 *
 *  Created on: Dec 17, 2019
 *      Author: dumitru
 */

#ifndef IEEE_802_15_4_PHY_CFG_H_
#define IEEE_802_15_4_PHY_CFG_H_


/***************************Imports*************************************************/
#include <stdint.h>
#include "ieee_802_15_4_phy.h"

/***************************Public Macro Definitions********************************/


/***************************Public Type Definitions********************************/

typedef void (*voidFuncPtr)(uint8_t*);
typedef void (*voidu8FuncPtr)(uint8_t);
typedef void (*voidu32FuncPtr)(uint32_t);

/***************************Public Data Definitions********************************/

/*!<
 *!< @brief PHY Pointers to Chip specific config and interfaces to upper layers
 *!< */
extern const voidFuncPtr DataRequestChipSpecificCallback;
extern const PD_Data_Confirm_t DataConfirmationCallback;
extern const PD_Data_Indication_t DataIndicationCallback;

extern const PLME_CCA_Request_t CcaRequestChipSpecificCallback;
extern const PLME_CCA_Confirm_t CcaConfirmCallback;

extern const PLME_ED_Request_t EdRequestChipSpecificCallback;
extern const PLME_ED_Confirm_t EdConfirmCallback;

extern const PLME_GET_Request_t GetRequestChipSpecificCallback;
extern const PLME_GET_Confirm_t GetConfirmCallback;

extern const PLME_SET_TRX_STATE_Request_t SetTrxStateRequestChipSpecificCallback;
extern const PLME_SET_TRX_STATE_Confirm_t SetTrxStateConfirmCallback;

extern const voidu8FuncPtr SetPhyChannelCallback;
extern const voidu32FuncPtr SetSupportedChannelsCallback;
extern const voidu8FuncPtr SetTransmitPowerCallback;
extern const voidu8FuncPtr SetCCAModeCallback;

extern const PLME_SET_Request_t SetRequestChipSpecificCallback;
extern const PLME_SET_Confirm_t SetConfirmCallback;

extern const IEEE_802_15_4_PHY_Enum_t (*GetTranceiverState)(void);


/***************************Public Function Definitions****************************/


#endif /* IEEE_802_15_4_PHY_CFG_H_ */
