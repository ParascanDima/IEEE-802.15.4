/*
 * ieee_802_15_4_phy.c
 *
 *  Created on: 8 ���. 2018 �.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ieee_802_15_4_phy.h"

/**************Private Macro Definitions*****************/


#define IS_CHANNEL_SUPPORTED(x)    (((1<<((uint8_t)x)) & phyChannelsSupported.value) != 0)

/**************Private Type Definitions******************/

typedef void (*voidFuncPtr)(uint8_t*);

/**************Private Variable Definitions**************/

/*  PHY PIB attribute */
uint8_t phyCurrentChannel;

/*  PHY PIB attribute */
IEEE_802_15_4_PhyChnDescr_t phyChannelsSupported;

/*  PHY PIB attribute */
IEEE_802_15_4_PhyTxPower_t phyTransmitPower;

/*  PHY PIB attribute */
uint8_t phyCCAMode;

/*!<
 *!< @brief PHY Pointers to Chip specific config and interfaces to upper layers
 *!< */
voidFuncPtr DataRequestChipSpecificCallback = NULL;
PD_Data_Confirm_t DataConfirmationCallback = NULL;
IEEE_802_15_4_PHY_Enum_t DataConfirmation = IDLE;
PD_Data_Indication_t DataIndicationCallback = NULL;

PLME_CCA_Request_t CcaRequestChipSpecificCallback = NULL;
PLME_CCA_Confirm_t CcaConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t CcaConfirmation = IDLE;

PLME_ED_Request_t EdRequestChipSpecificCallback = NULL;
PLME_ED_Confirm_t EdConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t EdConfirmation = IDLE;

PLME_GET_Request_t GetRequestChipSpecificCallback = NULL;
PLME_GET_Confirm_t GetConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t GetConfirmation = IDLE;

PLME_SET_TRX_STATE_Request_t SetTrxStateRequestChipSpecificCallback = NULL;
PLME_SET_TRX_STATE_Confirm_t SetTrxStateConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t SetTrxStateConfirmation = IDLE;

void (*SetPhyChannelCallback)(uint8_t) = NULL;
void (*SetSupportedChannelsCallback)(uint32_t) = NULL;
void (*SetTransmitPowerCallback)(uint8_t) = NULL;
void (*SetCCAModeCallback)(uint8_t) = NULL;

PLME_SET_Request_t SetRequestChipSpecificCallback = NULL;
PLME_SET_Confirm_t SetConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t SetConfirmation = IDLE;

IEEE_802_15_4_PHY_Enum_t (*GetTranceiverState)(void) = NULL;

/*!<
 *!< @brief PHY internal operating state of the transceiver
 *!< */
IEEE_802_15_4_PHY_Enum_t _IEEE_802_15_4_Intern_Transceiver_State_ = TRX_OFF;

/**************Public Variable Definitions***************/

IEEE_802_15_4_PHY_t phyMain;

/**************Private Function Definitions**************/

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyDataRequest
 *!< @brief                  : Requests the transfer of an MPDU (i.e., PSDU) from the MAC
 *!<                         : sublayer to the local PHY entity (per IEEE Std 802.15.4-2003   6.2.1.1 PD-DATA.request)
 *!< Parameters              :
 *!<                   Input : psduLength - The number of octets contained in the PSDU to be transmitted by the PHY entity.
 *!<                         : psdu - The set of octets forming the PSDU to be transmitted by the PHY entity.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_PhyDataRequest(uint8_t psduLength, IEEE_802_15_4_PSDU_t psdu)
{
    /* the instance of new packet should be created */
    IEEE_802_15_4_PPDU_t phyPacket = {.Preable = (uint32_t)0x00000000, .SFD = (uint8_t)0xA7};
    IEEE_802_15_4_PHY_Enum_t tranceiverState = IEEE_802_15_4_GetTranceiverState();

    if (TRX_OFF == tranceiverState)
    {
        if (psduLength <= aMaxPHYPacketSize)
        {
            phyPacket.frameLength = (psduLength << 1);
            phyPacket.PSDU = psdu;

            if (DataRequestChipSpecificCallback != NULL)
            {
                phyMain.PLME_SAP.SET_TRX_STATE.Request(TX_ON);
                if (TX_ON == SetTrxStateConfirmation)
                {
                    DataRequestChipSpecificCallback((uint8_t*)&phyPacket);
                }
            }
            DataConfirmation = TX_ON;
        }
        else
        {
            phyMain.PD_SAP.DATA.Confirm(RX_ON);
            DataConfirmation = RX_ON;
        }
    }
    else
    {
        phyMain.PD_SAP.DATA.Confirm(tranceiverState);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyDataConfirm
 *!< @brief                  : Confirms the end of the transmission of an MPDU (i.e., PSDU) from a
 *!<                         : local MAC sublayer entity to a peer MAC sublayer entity
 *!<                         : (per IEEE Std 802.15.4-2003   6.2.1.2 PD-DATA.confirm)
 *!< Parameters              :
 *!<                   Input : status - result of request
 *!<                   Output: -
 *!< Return                  : The result of the request to transmit a packet.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_PhyDataConfirm(IEEE_802_15_4_PHY_Enum_t status)
{
    phyMain.PLME_SAP.SET_TRX_STATE.Request(RX_ON);

    if (DataConfirmationCallback != NULL)
    {
        DataConfirmationCallback(status);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyDataIndication
 *!< @brief                  : Indicates the transfer of an MPDU (i.e., PSDU) from the PHY to the
 *!<                         : local MAC sublayer entity (per IEEE Std 802.15.4-2003   6.2.1.3 PD-DATA.indication)
 *!< Parameters              :
 *!<                   Input : psduLength - The number of octets contained in the PSDU received by the PHY entity.
 *!<                         : psdu - The set of octets forming the PSDU received by the PHY entity
 *!<                         : ppduLinkQuality - Link quality (LQ) value measured during reception of the PPDU.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_PhyDataIndication(uint8_t psduLength, uint8_t* psdu, uint8_t ppduLinkQuality)
{
    if (psduLength != 0 && psduLength <= aMaxPHYPacketSize)
    {
        if (DataIndicationCallback != NULL)
        {
            DataIndicationCallback(psduLength, psdu, ppduLinkQuality);
        }
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_CCA_Request
 *!< @brief                  : Requests that the PLME perform a CCA.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_CCA_Request(void)
{
    IEEE_802_15_4_PHY_Enum_t tranceiverState;

    tranceiverState = IEEE_802_15_4_GetTranceiverState();
    if (RX_ON == tranceiverState)
    {
        if (CcaRequestChipSpecificCallback != NULL)
        {
            CcaRequestChipSpecificCallback();
        }
    }
    else
    {
        phyMain.PLME_SAP.CCA.Confirm(tranceiverState);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_CCA_Confirm
 *!< @brief                  : Reports the results of a CCA.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : The result of the request to perform a CCA.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_CCA_Confirm(IEEE_802_15_4_PHY_Enum_t status)
{
    if (CcaConfirmCallback != NULL)
    {
        CcaConfirmCallback(status);
        CcaConfirmation = status;
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ED_Request
 *!< @brief                  : Requests that the PLME perform an ED measurement.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_ED_Request(void)
{
    IEEE_802_15_4_PHY_Enum_t tranceiverState;

    tranceiverState = IEEE_802_15_4_GetTranceiverState();
    if (RX_ON == tranceiverState)
    {
        if (EdRequestChipSpecificCallback != NULL)
        {
            EdConfirmation = IDLE;
            EdRequestChipSpecificCallback();
        }
    }
    else
    {
        phyMain.PLME_SAP.ED.Confirm(tranceiverState, 0);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ED_Confirm
 *!< @brief                  : Reports the results of the ED measurement.
 *!< Parameters              :
 *!<                   Input : EnergyLevel - ED level for the current channel
 *!<                   Output: -
 *!< Return                  : The result of the request to perform an ED measurement.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_ED_Confirm(IEEE_802_15_4_PHY_Enum_t status, uint8_t* EnergyLevel)
{
    if (EdConfirmCallback != NULL)
    {
        EdConfirmCallback(status, EnergyLevel);
    }
    EdConfirmation = status;
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GET_Request
 *!< @brief                  : Requests information about a given PHY PIB attribute.
 *!< Parameters              :
 *!<                   Input : PIBAttribID - The identifier of the PHY PIB attribute to get.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_GET_Request(IEEE_802_15_4_PIB_ID_t PIBAttribID)
{
    if (PHY_CCA_MODE >= PIBAttribID)
    {
        if (GetRequestChipSpecificCallback != NULL)
        {
            GetRequestChipSpecificCallback(PIBAttribID);
        }
    }
    else
    {
        phyMain.PLME_SAP.GET.Confirm(PHY_UNSUPPORTED_ATTRIBUTE, PIBAttribID, NULL);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GET_Confirm
 *!< @brief                  :
 *!< Parameters              :
 *!<                   Input : PIBAttribID - The identifier of the PHY PIB attribute to get.
 *!<                         : PIBAttributeValue - The value of the indicated PHY PIB attribute to get.
 *!<                   Output: -
 *!< Return                  : The result of the request for PHY PIB attribute information.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_GET_Confirm(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue)
{
    switch (PIBAttribID) {
        case PHY_CURRENT_CHANNEL:
            phyCurrentChannel = *PIBAttributeValue;
            break;
        case PHY_CHANNELS_SUPPORTED:
            phyChannelsSupported.value = *(uint32_t*)PIBAttributeValue;
            break;
        case PHY_TRANSMIT_POWER:
            phyTransmitPower.value = *PIBAttributeValue;
            break;
        case PHY_CCA_MODE:
            phyCCAMode = *PIBAttributeValue;
            break;
        default:
            break;
    }
    if (GetConfirmCallback != NULL)
    {
        GetConfirmCallback(status, PIBAttribID, PIBAttributeValue);
        GetConfirmation = status;
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_SET_TRX_STATE_Request
 *!< @brief                  : Requests that the PHY entity change the internal operating state of the transceiver.
 *!< Parameters              :
 *!<                   Input : state - The new state in which to configure the transceiver
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_SET_TRX_STATE_Request(IEEE_802_15_4_PHY_Enum_t state)
{
    if ((TX_ON == state) || (TRX_OFF == state) || (RX_ON == state) || (FORCE_TRX_OFF == state))
    {
        if (SetTrxStateRequestChipSpecificCallback != NULL)
        {
            SetTrxStateRequestChipSpecificCallback(state);
        }
    }
    else
    {
        phyMain.PLME_SAP.SET_TRX_STATE.Confirm(BUSY_RX);
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_SET_TRX_STATE_Confirm
 *!< @brief                  : Reports the result of a request to change the internal operating state of
 *!<                         : the transceiver.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : The result of the request to change the state of the transceiver.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_SET_TRX_STATE_Confirm(IEEE_802_15_4_PHY_Enum_t status)
{
    if (SetTrxStateConfirmCallback != NULL)
    {
        SetTrxStateConfirmCallback(status);
    }
    SetTrxStateConfirmation = status;
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_SET_Request
 *!< @brief                  : Is generated by the MLME and issued to its PLME to write the indicated
 *!<                         : PHY PIB attribute.
 *!< Parameters              :
 *!<                   Input : PIBAttribID - The identifier of the PIB attribute to set.
 *!<                         : PIBAttributeValue - The value of the indicated PIB attribute to set.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_SET_Request(IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue)
{

    switch (PIBAttribID)
    {
    case PHY_CURRENT_CHANNEL:

        if (SetPhyChannelCallback != NULL)
        {
            if ((uint8_t)*PIBAttributeValue <= 26)
            {
                if (phyChannelsSupported.value != 0 &&
                        IS_CHANNEL_SUPPORTED(*PIBAttributeValue))
                {
                    phyCurrentChannel = *PIBAttributeValue;
                    SetPhyChannelCallback(phyCurrentChannel);
                    phyMain.PLME_SAP.SET.Confirm(PHY_SUCCESS, PIBAttribID);
                }
                else
                {
                    phyMain.PLME_SAP.SET.Confirm(PHY_INVALID_PARAMETER, PIBAttribID);
                }
            }
            else
            {
                phyMain.PLME_SAP.SET.Confirm(PHY_INVALID_PARAMETER, PIBAttribID);
            }
        }

        break;
    case PHY_CHANNELS_SUPPORTED:
        if (IEEE_802_15_4_MAX_NUM_CHANNELS >= (uint8_t)*PIBAttributeValue)
        {
            if (SetSupportedChannelsCallback != NULL)
            {
                phyChannelsSupported.value = (1<<(uint8_t)*PIBAttributeValue);
                SetSupportedChannelsCallback(phyChannelsSupported.value);
                phyMain.PLME_SAP.SET.Confirm(PHY_SUCCESS, PIBAttribID);
            }
        }
        else
        {
            phyMain.PLME_SAP.SET.Confirm(PHY_INVALID_PARAMETER, PIBAttribID);
        }
        break;
    case PHY_TRANSMIT_POWER:
        if (SetTransmitPowerCallback != NULL)
        {
            if ((uint8_t)*PIBAttributeValue <= 0xBF)
            {
                phyTransmitPower.value = (uint8_t)*PIBAttributeValue;
                SetTransmitPowerCallback(phyTransmitPower.value);
                phyMain.PLME_SAP.SET.Confirm(PHY_SUCCESS, PIBAttribID);
            }
            else
            {
                phyMain.PLME_SAP.SET.Confirm(PHY_INVALID_PARAMETER, PIBAttribID);
            }
        }
        break;
    case PHY_CCA_MODE:

        if (SetCCAModeCallback != NULL)
        {
            if ((uint8_t)*PIBAttributeValue <= 0xBF)
            {
                phyCCAMode = *PIBAttributeValue;
                SetCCAModeCallback(phyCCAMode);
                phyMain.PLME_SAP.SET.Confirm(PHY_SUCCESS, PIBAttribID);
            }
            else
            {
                phyMain.PLME_SAP.SET.Confirm(PHY_INVALID_PARAMETER, PIBAttribID);
            }
        }
        break;
    default:
        phyMain.PLME_SAP.SET.Confirm(PHY_UNSUPPORTED_ATTRIBUTE, PIBAttribID);
        break;

    }
}



/****************************************************************************************
 *!< Function                : IEEE_802_15_4_SET_Confirm
 *!< @brief                  : Reports the results of the attempt to set a PIB attribute.
 *!< Parameters              :
 *!<                   Input : PIBAttribID - The identifier of the PIB attribute being confirmed.
 *!<                   Output: -
 *!< Return                  : The status of the attempt to set the request PIB attribute.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_SET_Confirm(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID)
{
    if (SetConfirmCallback != NULL)
    {
        SetConfirmCallback(status, PIBAttribID);
    }
    SetConfirmation = status;
}


/**************Public Function Definitions***************/

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyInit
 *!< @brief                  : Initialization of PHY layer
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_PhyInit(void)
{
    phyMain.PD_SAP.DATA.Request = IEEE_802_15_4_PhyDataRequest;
    phyMain.PD_SAP.DATA.Confirm = IEEE_802_15_4_PhyDataConfirm;
    phyMain.PD_SAP.DATA.Indication = IEEE_802_15_4_PhyDataIndication;


    phyMain.PLME_SAP.CCA.Request   = IEEE_802_15_4_CCA_Request;
    phyMain.PLME_SAP.CCA.Confirm   = IEEE_802_15_4_CCA_Confirm;

    phyMain.PLME_SAP.ED.Request   = IEEE_802_15_4_ED_Request;
    phyMain.PLME_SAP.ED.Confirm   = IEEE_802_15_4_ED_Confirm;

    phyMain.PLME_SAP.GET.Request   = IEEE_802_15_4_GET_Request;
    phyMain.PLME_SAP.GET.Confirm   = IEEE_802_15_4_GET_Confirm;

    phyMain.PLME_SAP.SET_TRX_STATE.Request   = (PLME_SET_TRX_STATE_Request_t)IEEE_802_15_4_SET_TRX_STATE_Request;
    phyMain.PLME_SAP.SET_TRX_STATE.Confirm   = IEEE_802_15_4_SET_TRX_STATE_Confirm;

    phyMain.PLME_SAP.SET.Request   = IEEE_802_15_4_SET_Request;
    phyMain.PLME_SAP.SET.Confirm   = IEEE_802_15_4_SET_Confirm;

}

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_BindService
 *!< @brief                  :
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_BindService(IEEE_802_15_4_ServiceAccessPoint_t serviceId, IEEE_802_15_4_Service_t serviceType, void (*func)(void*))
{
    if (func != NULL)
    {

        switch (serviceId)
        {
            case PD_DATA:

                switch (serviceType)
                {
                    case enRequest:
                        DataRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        DataConfirmationCallback = (PD_Data_Confirm_t)func;
                        break;

                    case enIndication:
                        DataIndicationCallback = (PD_Data_Indication_t)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_CCA:

                switch (serviceType)
                {
                    case enRequest:
                        CcaRequestChipSpecificCallback = (PLME_CCA_Request_t)func;
                        break;

                    case enConfirm:
                        CcaConfirmCallback = (PLME_CCA_Confirm_t)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_ED:

                switch (serviceType)
                {
                    case enRequest:
                        EdRequestChipSpecificCallback = (PLME_ED_Request_t)func;
                        break;

                    case enConfirm:
                        EdConfirmCallback = (PLME_ED_Confirm_t)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_GET:

                switch (serviceType)
                {
                    case enRequest:
                        GetRequestChipSpecificCallback = (PLME_GET_Request_t)func;
                        break;

                    case enConfirm:
                        GetConfirmCallback = (PLME_GET_Confirm_t)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_SET_TRX_STATE:

                switch (serviceType)
                {
                    case enRequest:
                        SetTrxStateRequestChipSpecificCallback = (PLME_SET_TRX_STATE_Request_t)func;
                        break;

                    case enConfirm:
                        SetTrxStateConfirmCallback = (PLME_SET_TRX_STATE_Confirm_t)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_SET:

                switch (serviceType)
                {
                    case enRequest:
                        SetRequestChipSpecificCallback = (PLME_SET_Request_t)func;
                        break;

                    case enConfirm:
                        SetConfirmCallback = (PLME_SET_Confirm_t)func;
                        break;

                    default:
                        break;
                }
                break;

            default:
            break;
        }

    }
    else
    {

    }

}



/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GetTranceiverState
 *!< @brief                  : Returns the state of tranceiver read from chip.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : State of the tranceiver
 *!< Critical section YES/NO : NO
 */
IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_GetTranceiverState(void)
{
    IEEE_802_15_4_PHY_Enum_t state;

    if (GetTranceiverState != NULL)
    {
        state = GetTranceiverState();
    }
    else
    {
        state = TRX_OFF;
    }
    _IEEE_802_15_4_Intern_Transceiver_State_ = state;

    return state;
}









