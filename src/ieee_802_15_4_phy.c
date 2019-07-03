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
 *!< @brief PHY constants
 *!< */
const uint8_t aMaxPHYPacketSize = 127;
const uint8_t aTurnaroundTime = 12;

/*!<
 *!< @brief PHY Pointers to Chip specific config
 *!< */
voidFuncPtr DataRequestChipSpecificCallback = NULL;
voidFuncPtr DataConfirmationCallback = NULL;
IEEE_802_15_4_PHY_Enum_t DataConfirmation = IDLE;
voidFuncPtr DataIndicationMacCallback = NULL;

voidFuncPtr CcaRequestChipSpecificCallback = NULL;
voidFuncPtr CcaConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t CcaConfirmation = IDLE;

voidFuncPtr EdRequestChipSpecificCallback = NULL;
voidFuncPtr EdConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t EdConfirmation = IDLE;

voidFuncPtr GetRequestChipSpecificCallback = NULL;
voidFuncPtr GetConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t GetConfirmation = IDLE;

voidFuncPtr SetTrxStateRequestChipSpecificCallback = NULL;
voidFuncPtr SetTrxStateConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t SetTrxStateConfirmation = IDLE;

void (*SetPhyChannelCallback)(uint8_t) = NULL;
void (*SetSupportedChannelsCallback)(uint32_t) = NULL;
void (*SetTransmitPowerCallback)(uint8_t) = NULL;
void (*SetCCAModeCallback)(uint8_t) = NULL;

voidFuncPtr SetRequestChipSpecificCallback = NULL;
voidFuncPtr SetConfirmCallback = NULL;
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
 *!<                         : confirmationCallback - Non standard parameter used to confirm data transfer
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_PhyDataRequest(uint8_t psduLength, IEEE_802_15_4_PSDU_t psdu, void (*confirmationCallback)(uint8_t*))
{
    /* the instance of new packet should be created */
    IEEE_802_15_4_PPDU_t phyPacket = {.Preable = (uint32_t)0x00000000, .SFD = (uint8_t)0xA7};

    phyMain.PLME_SAP.SET_TRX_STATE.Request(TX_ON);

    if (psduLength <= aMaxPHYPacketSize)
    {
        phyPacket.frameLength = (psduLength << 1);
        phyPacket.PSDU = psdu;

        if (DataRequestChipSpecificCallback != NULL){
            DataRequestChipSpecificCallback();
        }
        DataConfirmationCallback = confirmationCallback;
        DataConfirmation = TX_ON;
    }
    else
    {
        DataConfirmation = RX_ON;
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyDataConfirm
 *!< @brief                  : Confirms the end of the transmission of an MPDU (i.e., PSDU) from a
 *!<                         : local MAC sublayer entity to a peer MAC sublayer entity
 *!<                         : (per IEEE Std 802.15.4-2003   6.2.1.2 PD-DATA.confirm)
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : The result of the request to transmit a packet.
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_PhyDataConfirm(IEEE_802_15_4_PHY_Enum_t status)
{
    phyMain.PLME_SAP.SET_TRX_STATE.Request(RX_ON);
    //DataConfirmation = PHY_SUCCESS;

    if (DataConfirmationCallback != NULL)
    {
        DataConfirmationCallback((uint8_t*)&status);
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
        if (DataIndicationMacCallback != NULL)
        {
            DataIndicationMacCallback(psdu);
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
            CcaRequestChipSpecificCallback((uint8_t*)&CcaConfirmation);
        }
        else
        {
            CcaConfirmation = tranceiverState;
        }
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
        CcaConfirmCallback((uint8_t*)&status);
    }
    CcaConfirmation = status;
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
    if (EdRequestChipSpecificCallback != NULL)
    {
        EdRequestChipSpecificCallback();
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
        EdConfirmCallback();
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
    if (GetRequestChipSpecificCallback != NULL)
    {
        GetRequestChipSpecificCallback((uint8_t*)&PIBAttribID);
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
    if (GetConfirmCallback != NULL)
    {
        GetConfirmCallback();
    }
    GetConfirmation = status;
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
    if (SetTrxStateRequestChipSpecificCallback != NULL)
    {
        SetTrxStateRequestChipSpecificCallback();
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
        SetTrxStateConfirmCallback();
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
                    SetConfirmation = PHY_SUCCESS;
                }
                else
                {

                }
            }
            else
            {

            }
        }

        break;
    case PHY_CHANNELS_SUPPORTED:

        if (SetSupportedChannelsCallback != NULL)
        {
            phyChannelsSupported.value = (1<<(uint8_t)*PIBAttributeValue);
            SetSupportedChannelsCallback(phyChannelsSupported.value);
            SetConfirmation = PHY_SUCCESS;
        }
        else
        {

        }
        break;
    case PHY_TRANSMIT_POWER:
        if (SetTransmitPowerCallback != NULL)
        {
            if ((uint8_t)*PIBAttributeValue <= 0xBF)
            {
                phyTransmitPower.value = (uint8_t)*PIBAttributeValue;
                SetTransmitPowerCallback(phyTransmitPower.value);
                SetConfirmation = PHY_SUCCESS;
            }
            else
            {

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
                SetConfirmation = PHY_SUCCESS;
            }
            else
            {

            }
        }
        break;
    default:
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
        SetConfirmCallback();
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

    phyMain.PLME_SAP.SET_TRX_STATE.Request   = IEEE_802_15_4_SET_TRX_STATE_Request;
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
void IEEE_802_15_4_BindService(IEEE_802_15_4_ServiceAccessPoint_t serviceId, IEEE_802_15_4_Service_t serviceType, void (*func)(uint8_t*))
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
                        DataConfirmationCallback = (voidFuncPtr)func;
                        break;

                    case enIndication:
                        DataIndicationMacCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_CCA:

                switch (serviceType)
                {
                    case enRequest:
                        CcaRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        CcaConfirmCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_ED:

                switch (serviceType)
                {
                    case enRequest:
                        EdRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        EdConfirmCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_GET:

                switch (serviceType)
                {
                    case enRequest:
                        GetRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        GetConfirmCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_SET_TRX_STATE:

                switch (serviceType)
                {
                    case enRequest:
                        SetTrxStateRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        SetTrxStateConfirmCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_SET:

                switch (serviceType)
                {
                    case enRequest:
                        SetRequestChipSpecificCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        SetConfirmCallback = (voidFuncPtr)func;
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









