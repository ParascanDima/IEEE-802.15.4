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

typedef void (*voidFuncPtr)(void);

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


voidFuncPtr DataRequestCallback = NULL;
voidFuncPtr DataConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t DataConfirmation = IDLE;
voidFuncPtr DataIndicationCallback = NULL;

voidFuncPtr CcaRequestCallback = NULL;
voidFuncPtr CcaConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t CcaConfirmation = IDLE;

voidFuncPtr EdRequestCallback = NULL;
voidFuncPtr EdConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t EdConfirmation = IDLE;

voidFuncPtr GetRequestCallback = NULL;
voidFuncPtr GetConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t GetConfirmation = IDLE;

voidFuncPtr SetTrxStateRequestCallback = NULL;
voidFuncPtr SetTrxStateConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t SetTrxStateConfirmation = IDLE;

void (*SetPhyChannelCallback)(uint8_t) = NULL;
void (*SetSupportedChannelsCallback)(uint32_t) = NULL;
void (*SetTransmitPowerCallback)(uint8_t) = NULL;
void (*SetCCAModeCallback)(uint8_t) = NULL;

voidFuncPtr SetRequestCallback = NULL;
voidFuncPtr SetConfirmCallback = NULL;
IEEE_802_15_4_PHY_Enum_t SetConfirmation = IDLE;

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
    if (DataRequestCallback != NULL){
        DataRequestCallback();
    }
    DataConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_PhyDataConfirm(void)
{
    if (DataConfirmCallback != NULL)
    {
        DataConfirmCallback();
    }
    return DataConfirmation;
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
    if (DataIndicationCallback != NULL)
    {
        DataIndicationCallback();
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
    if (CcaRequestCallback != NULL)
    {
        CcaRequestCallback();
    }
    CcaConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_CCA_Confirm(void)
{
    if (CcaConfirmCallback != NULL)
    {
        CcaConfirmCallback();
    }
    return CcaConfirmation;
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
    if (EdRequestCallback != NULL)
    {
        EdRequestCallback();
    }
    EdConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_ED_Confirm(uint8_t* EnergyLevel)
{
    if (EdConfirmCallback != NULL)
    {
        EdConfirmCallback();
    }
    return EdConfirmation;
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
    if (GetRequestCallback != NULL)
    {
        GetRequestCallback();
    }
    GetConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_GET_Confirm(IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue)
{
    if (GetConfirmCallback != NULL)
    {
        GetConfirmCallback();
    }
    return GetConfirmation;
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
    if (SetTrxStateRequestCallback != NULL)
    {
        SetTrxStateRequestCallback();
    }
    SetTrxStateConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_SET_TRX_STATE_Confirm(void)
{
    if (SetTrxStateConfirmCallback != NULL)
    {
        SetTrxStateConfirmCallback();
    }
    return SetTrxStateConfirmation;
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
                    SetConfirmation = SUCCESS;
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
            SetConfirmation = SUCCESS;
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
                SetConfirmation = SUCCESS;
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
                SetConfirmation = SUCCESS;
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
static IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_SET_Confirm(IEEE_802_15_4_PIB_ID_t PIBAttribID)
{
    if (SetConfirmCallback != NULL)
    {
        SetConfirmCallback();
    }
    return IDLE;
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
void IEEE_802_15_4_BindService(IEEE_802_15_4_ServiceAccessPoint_t serviceId, IEEE_802_15_4_Service_t serviceType, void (*func)(void))
{
    if (func != NULL)
    {

        switch (serviceId)
        {
            case PD_DATA:

                switch (serviceType)
                {
                    case enRequest:
                        DataRequestCallback = (voidFuncPtr)func;
                        break;

                    case enConfirm:
                        DataConfirmCallback = (voidFuncPtr)func;
                        break;

                    case enIndication:
                        DataIndicationCallback = (voidFuncPtr)func;
                        break;

                    default:
                        break;
                }
                break;

            case PLME_CCA:

                switch (serviceType)
                {
                    case enRequest:
                        CcaRequestCallback = (voidFuncPtr)func;
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
                        EdRequestCallback = (voidFuncPtr)func;
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
                        GetRequestCallback = (voidFuncPtr)func;
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
                        SetTrxStateRequestCallback = (voidFuncPtr)func;
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
                        SetRequestCallback = (voidFuncPtr)func;
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













