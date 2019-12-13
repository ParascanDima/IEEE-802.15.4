/*
 * ieee_802_15_4_mac.c
 *
 *  Created on: 8 ���. 2018 �.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ieee_802_15_4_mac.h"

/**************Private Macro Definitions*****************/

#define IEEE_802_15_4_MAC_MAX_PEND_TRANSACTIONS            (uint8_t)0x0A
#define IEEE_802_15_4_MAC_FIRST_IN_QUEUE                   (&transactionQueue[0])

#define IEEE_802_15_4_MAC_DATA_ACK_REQUIRED                true
#define IEEE_802_15_4_MAC_DATA_ACK_NOT_REQUIRED            false

#define IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX            (uint8_t)2
#define IEEE_802_15_4_MAC_BEACON_SRC_PANID_INDEX           (uint8_t)3


/**************Private Type Definitions******************/
/*!<
 *!< @brief Is not a part of IEEE Std 802.15.4, defined for MAC task to manage the MAC functionality
 *!< */
typedef struct
{
    uint32_t u32MacTaskCount;
    uint8_t  transactionsInQueue;
}IEEE_802_15_4_MAC_GeneralState_t;

/*!<
 *!< @brief Transaction type is not described in IEEE Std 802.15.4, but is necessary for determining the errors for each transaction
 *!< */
typedef struct
{
    uint32_t transactionTime;              /* Time of transaction in ticks of MAC task for detecting timeouts                                 */
    uint8_t msduHandle;                     /* Number which is generated on each transaction                                                   */
    uint8_t transactionsType;              /* Describes if the transaction was data or MAC command                                            */
    bool isAckRequired;                    /* Describes if the acknowledge is required for this transaction for determining transaction error */
}IEEE_802_15_4_PendTransaction_t;

/**************Private Variable Definitions**************/

/*!<
 *!< @brief MAC general state
 *!< */
static IEEE_802_15_4_MAC_GeneralState_t IEEE_802_15_4_Mac_State =
        {
         .transactionsInQueue = 0,
         .u32MacTaskCount = 0,
        };


/*!<
 *!< @brief Transaction Queue is not described in IEEE Std 802.15.4 standard
 *!< */
static IEEE_802_15_4_PendTransaction_t transactionQueue[IEEE_802_15_4_MAC_MAX_PEND_TRANSACTIONS];

/*!<
 *!< @brief Constants that are not defined by the IEEE Std 802.15.4-2003 but are upper side of threshold
 *!< */
const uint8_t ACLSecurityMaterialLengthMax = (uint8_t)26;
const uint8_t macACLEntryDescriptorSetSizeMax = UINT8_MAX;

/*!<
 *!< @brief MAC PIB attributes
 *!< */
static uint8_t macAckWaitDuration = (uint8_t)54;
static bool macAssociationPermit = (bool)false;
static bool macAutoRequest = (bool)true;
static bool macBattLifeExt = (bool)false;
static uint8_t macBattLifeExtPeriods = (uint8_t)6;
static uint8_t *macBeaconPayload = NULL;
static uint8_t macBeaconPayloadLength = (uint8_t)0;
static uint8_t macBeaconOrder = (uint8_t)15;
static uint32_t macBeaconTxTime = (uint32_t)0x00;
static uint8_t macBSN;
static uint64_t macCoordExtendedAddress;
static uint16_t macCoordShortAddress = (uint16_t)0xFFFF;
static uint8_t macDSN;
static bool macGTSPermit = (bool)true;
static uint8_t macMaxCSMABackoffs = 4;
static uint8_t macMinBE = 3;
static uint16_t macPANId = (uint16_t)0xFFFF;
static bool macPromiscuousMode = (bool)false;
static bool macRxOnWhenIdle = (bool)false;
static uint16_t macShortAddress = (uint16_t)0xFFFF;
static uint8_t macSuperframeOrder = (uint8_t)15;
static uint16_t macTransactionPersistenceTime = (uint16_t)0x01F4;

/*!<
 *!< @brief MAC PIB security attributes
 *!< */
static uint8_t *macACLEntryDescriptorSet = NULL;
static uint8_t macACLEntryDescriptorSetSize = (uint8_t)0x00;
static bool macDefaultSecurity = (bool)false;
static uint8_t macDefaultSecurityMaterialLength = (uint8_t)0x15;
static uint8_t macDefaultSecurityMaterial[ACLSecurityMaterialLengthMax] = "";
static uint8_t macDefaultSecuritySuite = (uint8_t)0x00;
static uint8_t macSecurityMode = (uint8_t)0x00;

/*!<
 *!< @brief Elements of ACL entry descriptor
 *!< */
static uint64_t ACLExtendedAddress;
static uint16_t ACLShortAddress = (uint16_t)0xFFFF;
static uint16_t ACLPANId;
static uint8_t ACLSecurityMaterialLength = (uint8_t)21;
static uint8_t ACLSecurityMaterial[ACLSecurityMaterialLengthMax] = "";
static uint8_t ACLSecuritySuite = (uint8_t)0x00;

/*!<
 *!< @brief MAC interfaces to upper layers
 *!< */



/**************Public Variable Definitions***************/

const uint64_t aExtendedAddress __attribute__((weak)) = (uint64_t)0xFFFFFFFFFFFFFFFF;

/**************Private Function Definitions**************/


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_AddPendingTransaction
 *!< @brief                  : Adding the transaction to pending queue
 *!<                         :
 *!< Parameters              :
 *!<                   Input : msduHandle       -
 *!<                         : transactionsType -
 *!<                         : isAckRequired    -
 *!<                         :
 *!<                   Output: -
 *!< Return                  : Return the MSDU handler number.
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_AddPendingTransaction(uint8_t msduHandle, uint8_t transactionsType, bool isAckRequired)
{
    uint8_t u8TrnsCount;

    u8TrnsCount = IEEE_802_15_4_Mac_State.transactionsInQueue;

    if (IEEE_802_15_4_MAC_MAX_PEND_TRANSACTIONS < u8TrnsCount)
    {
        transactionQueue[u8TrnsCount].transactionsType = transactionsType;
        transactionQueue[u8TrnsCount].isAckRequired = isAckRequired;

        transactionQueue[u8TrnsCount].msduHandle = msduHandle;
        transactionQueue[u8TrnsCount].transactionTime = IEEE_802_15_4_Mac_State.u32MacTaskCount;

    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_RemovePendingTransaction
 *!< @brief                  : Removing the transaction to pending queue
 *!<                         :
 *!< Parameters              :
 *!<                   Input : msduHandle       -
 *!<                         :
 *!<                   Output: -
 *!< Return                  : Return the MSDU handler number.
 *!< Critical section YES/NO : NO
 */
void IEEE_802_15_4_RemovePendingTransaction(uint8_t msduHandle)
{
    uint8_t u8IndexLcl;
    bool msduHandleFoundLcl = false;

    for (u8IndexLcl = 0; (u8IndexLcl < IEEE_802_15_4_Mac_State.transactionsInQueue); ++u8IndexLcl)
    {
        if (msduHandleFoundLcl == false)
        {
            msduHandleFoundLcl = (transactionQueue[u8IndexLcl].msduHandle == msduHandle);
        }
        else
        {
            transactionQueue[u8IndexLcl] = transactionQueue[u8IndexLcl+1];
        }
    }

    if ((IEEE_802_15_4_Mac_State.transactionsInQueue != 0) && (msduHandleFoundLcl == true))
    {
        IEEE_802_15_4_Mac_State.transactionsInQueue--;
    }
}

/**************Public Function Definitions***************/

/* Interfaces to PHY layer */

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
void IEEE_802_15_4_ToMacDataConfirmation(IEEE_802_15_4_PHY_Enum_t status)
{
    uint8_t locMsduHndl = IEEE_802_15_4_MAC_FIRST_IN_QUEUE->msduHandle;
    GeneralMacRequestStatusType reqStatus;

    switch(status)
    {
    case PHY_SUCCESS:
        reqStatus = SUCCESS;
        break;
    case RX_ON:
        reqStatus = CHANNEL_ACCESS_FAILURE;
        break;
    case TRX_OFF:
        reqStatus = CHANNEL_ACCESS_FAILURE;
        break;
    default:
        reqStatus = INVALID_PARAMETER;
        break;
    }

    IEEE_802_15_4_MacDataConfirm(locMsduHndl, reqStatus);
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_ToMacDataIndication
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
void IEEE_802_15_4_ToMacDataIndication(uint8_t psduLength, uint8_t* psdu, uint8_t ppduLinkQuality)
{
    IEEE_802_15_4_FrameCtrl   locFrameControl;

    /* Define the MHR size with minimum size of 3 bytes (2 bytes are frame control and 1 byte is sequence number) */
    uint8_t locMhrSize = 3;
    /* MFR size is constant for data frame */
    const uint8_t locMfrSize = 2;
    /* Read first 2 bytes of frame as these are frame control bits */
    locFrameControl.value = (uint16_t)((psdu[0] << 8) | psdu[1]);

    switch (locFrameControl.Field.frameType)
    {
    case IEEE_802_15_4_FRAME_TYPE_BEACON:
    {
        /* New beacon frame */
        IEEE_802_15_4_MacBeaconFrame_t locBcnFrame;
        /* Copy frame control to new frame */
        locBcnFrame.frameControl.value = locFrameControl.value;
        /* Read sequence counter from received frame */
        locBcnFrame.sequenceNumber = psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX];
        /* The addressing fields shall comprise only source address fields
         * PANID has static length just address mode should be determined */
        locBcnFrame.srcPANID = (uint16_t)((psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX+1] << 8) |
                psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX+2]);
        locMhrSize += 2;
        switch(locBcnFrame.frameControl.Field.srcAddrMode)
        {
        case IEEE_802_15_4_MAC_SHORT_ADDR_FIELD:
            locMhrSize += 2;
            locBcnFrame.srcAddr
            break;
        case IEEE_802_15_4_MAC_LONG_ADDR_FIELD:
            break;
        }


    }
    break;
    case IEEE_802_15_4_FRAME_TYPE_ACK:
    {
        /* New acknoledge frame */
        IEEE_802_15_4_MacAcknowledgeFrame_t locAckFrame;
        /* Copy frame control to new frame */
        locAckFrame.frameControl.value = locAckFrameControl.value;
        /* Read sequence counter from received frame */
        locAckFrame.sequenceNumber = psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX];

        /* ToDo: Functionality for reset timeouts */
    }
    break;
    case IEEE_802_15_4_FRAME_TYPE_MAC_COMMAND:
    {
        /* New command frame */
        IEEE_802_15_4_MacCommandFrame_t locCmdFrame;
        /* Copy frame control to new frame */
        locCmdFrame.frameControl.value = locFrameControl.value;
        /* Read sequence counter from received frame */
        locCmdFrame.sequenceNumber = psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX];

    }
    break;
    case IEEE_802_15_4_FRAME_TYPE_DATA:
    {
        /* New data frame */
        IEEE_802_15_4_MacDataFrame_t locDataFrame;

        /* Copy frame control to new frame */
        locDataFrame.frameControl.value = locDataFrameControl.value;
        /* Read sequence counter from received frame */
        locDataFrame.sequenceNumber = psdu[IEEE_802_15_4_MAC_SEQUENCE_NUMBER_INDEX];

        /* Check if frame is an intra PAN frame and either PAN ID is included in this frame or not */
        if (true == locDataFrame.frameControl.Field.intraPAN)
        {
            /* The PAN ID is resent in this frame it means that MHR is at least 2 bytes longer */
            locDataFrame.dstPANID = (uint16_t)((psdu[locMhrSize] << 8) | psdu[locMhrSize+1]);
            locMhrSize += 2;
        }
        else
        {
            /* Frame was sent by device in the same PAN */
            locDataFrame.dstPANID = 0x00;
        }
        /* As there are 3 cases of destination addressing modes each mode should be analyzed */
        switch (locDataFrame.frameControl.Field.dstAddrMode)
        {
        case IEEE_802_15_4_MAC_NO_ADDR_FIELD:
            /* No addressing mode is selected so the destination address is not present in this frame */
            locDataFrame.dstAddr = 0x00;
            break;
        case IEEE_802_15_4_MAC_SHORT_ADDR_FIELD:
            /* The short addressing mode is selected for this frame so the destination address is saved and MHR size is now 2 bytes longer */
            locDataFrame.dstAddr = (uint64_t)((uint16_t)((psdu[locMhrSize] << 8) | psdu[locMhrSize+1]));
            locMhrSize += 2;
            break;
        case IEEE_802_15_4_MAC_LONG_ADDR_FIELD:
            /* The long addressing mode is selected for this frame so the destination address is saved and MHR size is now 8 bytes longer */
            locDataFrame.dstAddr = (uint64_t)((psdu[locMhrSize] << (7*8)) | (psdu[locMhrSize+1] << (6*8)) | (psdu[locMhrSize+2] << (5*8)) |
                    (psdu[locMhrSize+3] << (4*8)) | (psdu[locMhrSize+4] << (3*8)) | (psdu[locMhrSize+5] << (2*8)) | (psdu[locMhrSize+6] << (1*8)) | psdu[locMhrSize+7] );
            locMhrSize += 8;
            break;
        default:
            break;
        }
        /* Reverse check if an intra PAN is not set then source PAN ID is present in  this frame */
        if (false == locDataFrame.frameControl.Field.intraPAN)
        {
            /* The PAN ID is resent in this frame it means that MHR is at least 2 bytes longer */
            locDataFrame.srcPANID = (uint16_t)((psdu[locMhrSize] << 8) | psdu[locMhrSize+1]);
            locMhrSize += 2;
        }
        else
        {
            /* Frame was sent by device in the same PAN */
            locDataFrame.srcPANID = 0x00;
        }
        /* Same 3 ways for source addressing mode */
        switch (locDataFrame.frameControl.Field.srcAddrMode)
        {
        case IEEE_802_15_4_MAC_NO_ADDR_FIELD:
            /* No addressing mode is selected so the source address is not present in this frame */
            locDataFrame.srcAddr = 0x00;
            break;
        case IEEE_802_15_4_MAC_SHORT_ADDR_FIELD:
            /* The short addressing mode is selected for this frame so the source address is saved and MHR size is now 2 bytes longer */
            locDataFrame.srcAddr = (uint64_t)((uint16_t)((psdu[locMhrSize] << 8) | psdu[locMhrSize+1]));
            locMhrSize += 2;
            break;
        case IEEE_802_15_4_MAC_LONG_ADDR_FIELD:
            /* The long addressing mode is selected for this frame so the source address is saved and MHR size is now 8 bytes longer */
            locDataFrame.srcAddr = (uint64_t)((psdu[locMhrSize] << (7*8)) | (psdu[locMhrSize+1] << (6*8)) | (psdu[locMhrSize+2] << (5*8)) |
                    (psdu[locMhrSize+3] << (4*8)) | (psdu[locMhrSize+4] << (3*8)) | (psdu[locMhrSize+5] << (2*8)) | (psdu[locMhrSize+6] << (1*8)) | psdu[locMhrSize+7] );
            locMhrSize += 8;
            break;
        default:
            break;
        }
        /* Now the meta data parsing finished, time to indicate upper layer (MAC) of incomming data */
        IEEE_802_15_4_MacDataIndication(locDataFrame.frameControl.Field.srcAddrMode,
                                        locDataFrame.srcPANID,
                                        locDataFrame.srcAddr,
                                        locDataFrame.frameControl.Field.dstAddrMode,
                                        locDataFrame.dstPANID,
                                        locDataFrame.dstAddr,
                                        (psduLength - locMhrSize - locMfrSize),
                                        &psdu[locMhrSize],
                                        ppduLinkQuality,
                                        locDataFrame.frameControl.Field.securityEnabled,
                                        0x08/* Temporar stub */);
    }
    }


}


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
void IEEE_802_15_4_ToMacCcaConfirmation(IEEE_802_15_4_PHY_Enum_t status)
{

}


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
void IEEE_802_15_4_ToMacEdConfirmation(IEEE_802_15_4_PHY_Enum_t status, uint8_t* EnergyLevel)
{

}


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
void IEEE_802_15_4_ToMacGetConfirmation(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue)
{

}


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
void IEEE_802_15_4_ToMacSetTrxStateConfirmation(IEEE_802_15_4_PHY_Enum_t status)
{

}


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
void IEEE_802_15_4_ToMacSetConfirmation(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID)
{

}



/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacDataRequest
 *!< @brief                  : Requests the transfer of a data SPDU (i.e., MSDU) from a local SSCS
 *!<                         : entity to a single peer SSCS entity.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : SrcAddrMode - The source addressing mode for this primitive and subsequent MPDU. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : SrcPANId - The 16 bit PAN identifier of the entity from which the MSDU is being transferred.
 *!<                         : SrcAddr - The individual device address of the entity from which the MSDU is being transferred.
 *!<                         : DstAddrMode - The destination addressing mode for this primitive and subsequent MPDU. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : DstPANId - The 16 bit PAN identifier of the entity to which the MSDU is being transferred.
 *!<                         : DstAddr - The individual device address of the entity to which the MSDU is being transferred.
 *!<                         : msduLength - The number of octets contained in the MSDU to be transmitted by the MAC sublayer entity.
 *!<                         : msdu - The set of octets forming the MSDU to be transmitted by the MAC sublayer entity.
 *!<                         : msduHandle - The handle associated with the MSDU to be transmitted by the MAC sublayer entity.
 *!<                         : TxOptions - The transmission options for this MSDU. These are a bitwise OR of one or more of the following:
 *!<                         :                  0 x 01 = acknowledged transmission.
 *!<                         :                  0 x 02 = GTS transmission.
 *!<                         :                  0 x 04 = indirect transmission.
 *!<                         :                  0 x 08 = security enabled transmission.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDataRequest(uint8_t SrcAddrMode,
                                         uint16_t SrcPANId,
                                         uint64_t SrcAddr,
                                         uint8_t DstAddrMode,
                                         uint16_t DstPANId,
                                         uint64_t DstAddr,
                                         uint8_t msduLength,
                                         uint8_t* msdu,
                                         uint8_t msduHandle,
                                         uint8_t TxOptions)
{
    uint8_t index;
    /* Create a new instance of packet for transmition */
    uint8_t dataFrameBytes[aMaxMACFrameSize];
    IEEE_802_15_4_FrameCtrl lFrmCtrl;
    uint8_t lDataShift = 0;
    uint8_t lDataSize = 0;

    /* Specify that the request is the data request */
    lFrmCtrl.Field.frameType = IEEE_802_15_4_FRAME_TYPE_DATA;

    if ((TxOptions & (uint8_t)0x0F) != (uint8_t)0x00)
    {
        if((TxOptions & IEEE_802_15_4_ACK_TRANSMISSION) != (uint8_t)0x00)
        {
            lFrmCtrl.Field.ackRequest = (uint8_t)0x01;
        }
        if((TxOptions & IEEE_802_15_4_GTS_TRANSMISSION) != (uint8_t)0x00)
        {
            lFrmCtrl.Field.ackRequest = (uint8_t)0x01;
        }
        else if((TxOptions & IEEE_802_15_4_INDIR_TRANSMISSION) != (uint8_t)0x00)
        {
            lFrmCtrl.Field.intraPAN = (uint8_t)0x01;
        }
        if((TxOptions & IEEE_802_15_4_SE_TRANSMISSION) != (uint8_t)0x00)
        {
            lFrmCtrl.Field.securityEnabled = (uint8_t)0x01;
        }
        /* Addressing modes are set here for source and destination */
        if ((uint8_t)0x01 != SrcAddrMode && SrcAddrMode < (uint8_t)0x05)
        {
            lFrmCtrl.Field.srcAddrMode = SrcAddrMode;
        }
        else
        {
            /* The review of this is required */
            lFrmCtrl.Field.srcAddrMode = IEEE_802_15_4_NO_ADDR_FIELD;
        }
        if ((uint8_t)0x01 != DstAddrMode && DstAddrMode < (uint8_t)0x05)
        {
            lFrmCtrl.Field.dstAddrMode = DstAddrMode;
        }
        else
        {
            /* The review of this is required */
            lFrmCtrl.Field.dstAddrMode = IEEE_802_15_4_NO_ADDR_FIELD;
        }
        /* Copy frame control to data buffer */
        dataFrameBytes[lDataShift++] = (uint8_t)(lFrmCtrl.value>>8);
        dataFrameBytes[lDataShift++] = (uint8_t)(lFrmCtrl.value&0xFF);

        dataFrameBytes[lDataShift++] = macDSN++;

        switch (lFrmCtrl.Field.dstAddrMode)
        {
        case IEEE_802_15_4_MAC_SHORT_ADDR_FIELD:
            /* Copying the PAN ID */
            dataFrameBytes[lDataShift++] = (uint8_t)(DstPANId>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstPANId&0xFF);
            /* Copying the short address */
            dataFrameBytes[lDataShift++] = (uint8_t)((uint16_t)DstAddr>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)((uint16_t)DstAddr&0xFF);
            break;
        case IEEE_802_15_4_MAC_LONG_ADDR_FIELD:
            /* Copying the PAN ID */
            dataFrameBytes[lDataShift++] = (uint8_t)(DstPANId>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstPANId&0xFF);
            /* Copying the long address */
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>56);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>48);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>40);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>32);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>24);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>16);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(DstAddr&0xFF);
            break;
        case IEEE_802_15_4_MAC_NO_ADDR_FIELD:
        default:
            break;
        }

        switch (lFrmCtrl.Field.srcAddrMode)
        {
        case IEEE_802_15_4_MAC_SHORT_ADDR_FIELD:
            /* Copying the PAN ID */
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcPANId>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcPANId&0xFF);
            /* Copying the short address */
            dataFrameBytes[lDataShift++] = (uint8_t)((uint16_t)SrcAddr>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)((uint16_t)SrcAddr&0xFF);
            break;
        case IEEE_802_15_4_MAC_LONG_ADDR_FIELD:
            /* Copying the PAN ID */
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcPANId>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcPANId&0xFF);
            /* Copying the long address */
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>56);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>48);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>40);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>32);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>24);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>16);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr>>8);
            dataFrameBytes[lDataShift++] = (uint8_t)(SrcAddr&0xFF);
            break;
        case IEEE_802_15_4_MAC_NO_ADDR_FIELD:
        default:
            break;
        }

        for (index = 0; index < msduLength; ++index) {
            dataFrameBytes[lDataShift+index] = msdu[index];
        }

        lDataSize = lDataShift+1;

#if IEEE_802_15_4_CRC_MANUAL_CALC == true
        /* ToDo: Implementation of FCS calculation */
        dataFrameBytes[lDataShift] = IEEE_802_15_4_MAC_FCS_Calculation(&dataFrameBytes[0], lDataSize);
#endif

        (void)IEEE_802_15_4_AddPendingTransaction(msduHandle, IEEE_802_15_4_FRAME_TYPE_DATA, (bool)lFrmCtrl.Field.ackRequest);

        phyMain.PD_SAP.DATA.Request(lDataSize, (IEEE_802_15_4_PSDU_t)&dataFrameBytes[0]);

    }
    else
    {
        /* The TxOptions are wrong */
    }
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacDataConfirm
 *!< @brief                  : Reports the results of a request to transfer a data SPDU (MSDU) from a
 *!<                         : local SSCS entity to a single peer SSCS entity.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : msduHandle - The handle associated with the MSDU being confirmed.
 *!<                         : status - The status of the last MSDU transmission.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDataConfirm(uint8_t msduHandle,
                                         GeneralMacRequestStatusType status)
{
    IEEE_802_15_4_RemovePendingTransaction(msduHandle);
}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacDataIndication
 *!< @brief                  : Indicates the transfer of a data SPDU (i.e., MSDU) from the MAC
 *!<                         : sublayer to the local SSCS entity.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : SrcAddrMode - The source addressing mode for this primitive and subsequent MPDU. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : SrcPANId - The 16 bit PAN identifier of the entity from which the MSDU is being transferred.
 *!<                         : SrcAddr - The individual device address of the entity from which the MSDU is being transferred.
 *!<                         : DstAddrMode - The destination addressing mode for this primitive and subsequent MPDU. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : DstPANId - The 16 bit PAN identifier of the entity to which the MSDU is being transferred.
 *!<                         : DstAddr - The individual device address of the entity to which the MSDU is being transferred.
 *!<                         : msduLength - The number of octets contained in the MSDU to be transmitted by the MAC sublayer entity.
 *!<                         : msdu - The set of octets forming the MSDU to be transmitted by the MAC sublayer entity.
 *!<                         : mpduLinkQuality - LQ value measured during reception of the MPDU. Lower values represent lower LQ (per IEEE Std 802.15.4-2003 6.7.8).
 *!<                         : SecurityUse - An indication of whether the received data frame is using security. This value is set to TRUE if the
 *!<                         :               security enable subfield was set to 1 or FALSE if the security enabled subfield was set to 0.
 *!<                         : ACLEntry - The macSecurityMode parameter value from the ACL entry associated with the sender of the data frame.
 *!<                         :            This value is set to 0 x 08 if the sender of the data frame was not found in the ACL.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDataIndication(uint8_t SrcAddrMode,
                                            uint16_t SrcPANId,
                                            uint64_t SrcAddr,
                                            uint8_t DstAddrMode,
                                            uint16_t DstPANId,
                                            uint64_t DstAddr,
                                            uint8_t msduLength,
                                            uint8_t* msdu,
                                            uint8_t mpduLinkQuality,
                                            bool SecurityUse,
                                            uint8_t ACLEntry)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacPurgeRequest
 *!< @brief                  : Allows the next higher layer to purge an MSDU from the transaction queue.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : msduHandle - The handle of the MSDU to be purged from the transaction queue.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacPurgeRequest(uint8_t msduHandle)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacPurgeConfirm
 *!< @brief                  : Allows the MAC sublayer to notify the next higher layer of the success of its
 *!<                         : request to purge an MSDU from the transaction queue.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : msduHandle - The handle of the MSDU requested to be purge from the transaction queue.
 *!<                         : status - The status of the request to be purged an MSDU from the transaction queue.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacPurgeConfirm(uint8_t msduHandle,
                                          GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacAssociateRequest
 *!< @brief                  : Define how a device becomes associated with a PAN.
 *!<                         : All devices shall provide an interface for the request and confirm association primitives. The indication and
 *!<                         : response association primitives are optional for an RFD.
 *!< Parameters              :
 *!<                   Input : LogicalChannel - The logical channel on which to attempt association.
 *!<                         : CoordAddrMode - The coordinator addressing mode for this primitive and subsequent MPDU. This value
 *!<                         :                 can take one of the following values:
 *!<                         :                      0x02=16 bit short address.
 *!<                         :                      0x03=64 bit extended address.
 *!<                         : CoordPANId - The identifier of the PAN with which to associate.
 *!<                         : CoordAddress - The address of the coordinator with which to associate.
 *!<                         : CapabilityInformation - Specifies the operational capabilities of the associating device.
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacAssociateRequest(uint8_t LogicalChannel,
                                              uint8_t CoordAddrMode,
                                              uint16_t CoordPANId,
                                              uint64_t CoordAddress,
                                              uint8_t CapabilityInformation,
                                              bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacAssociateIndication
 *!< @brief                  : Used to indicate the reception of an association request command.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : DeviceAddress - The address of the device requesting association.
 *!<                         : CapabilityInformation - The operational capabilities of the device requesting association.
 *!<                         : SecurityUse - An indication of whether the received MAC command frame is using security. This value is
 *!<                         :               set to TRUE if the security enable subfield was set to 1 or FALSE if the security enabled
 *!<                         :               subfield was set to 0.
 *!<                         : ACLEntry - The macSecurityMode parameter value from the ACL entry associated with the sender of the
 *!<                         :            data frame. This value is set to 0 x 08 if the sender of the data frame was not found in the ACL.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacAssociateIndication(uint64_t DeviceAddress,
                                                 uint8_t CapabilityInformation,
                                                 bool SecurityUse,
                                                 uint8_t ACLEntry)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacAssociateResponse
 *!< @brief                  : Used to initiate a response to an MLME-ASSOCIATE.indication primitive.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : DeviceAddress - The address of the device requesting association.
 *!<                         : AssocShortAddress - The short device address allocated by the coordinator on successful association.
 *!<                         :                     This parameter is set to 0xffff if the association was unsuccessful.
 *!<                         : status - The status of the association attempt.
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacAssociateResponse(uint64_t DeviceAddress,
                                               uint8_t AssocShortAddress,
                                               GeneralMacRequestStatusType status,
                                               bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacAssociateConfirm
 *!< @brief                  : Used to inform the next higher layer of the initiating device whether its request to associate was
 *!<                         : successful or unsuccessful.
 *!<                         :
 *!< Parameters              :
 *!<                   Input : AssocShortAddress - The short device address allocated by the coordinator on successful association.
 *!<                         :                     This parameter is set to 0xffff if the association was unsuccessful.
 *!<                         : status - The status of the association attempt.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacAssociateConfirm(uint16_t AssocShortAddress,
                                               GeneralMacRequestStatusType status)
{

}


/*************************************************************************NO_ACK***************
 *!< Function                : IEEE_802_15_4_MacDisassociateRequest
 *!< @brief                  : Define how a device becomes associated with a PAN.
 *!<                         : Used by an associated device to notify the coordinator of its intent to leave the PAN.
 *!<                         : It is also used by the coordinator to instruct an associated device to leave the PAN.
 *!< Parameters              :
 *!<                   Input : DeviceAddress - The address of the device to which to send the disassociation notification command.
 *!<                         : DisassociateReason - The reason for the disassociation
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDisassociateRequest(uint64_t DeviceAddress,
                                              uint8_t DisassociateReason,
                                              uint16_t SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacDisassociateIndication
 *!< @brief                  : Used to indicate the reception of a disassociation notification command.
 *!< Parameters              :
 *!<                   Input : DeviceAddress - The address of the device to which to send the disassociation notification command.
 *!<                         : DisassociateReason - The reason for the disassociation
 *!<                         : SecurityEnable - An indication of whether the received MAC command frame is using security. This value
 *!<                         :                  is set to TRUE if the security enable subfield was set to 1 or FALSE if the security
 *!<                         :                  enabled subfield was set to 0.
 *!<                         : ACLEntry - The macSecurityMode parameter value from the ACL entry associated with the sender of
 *!<                         :            the data frame. This value is set to 0x08 if the sender of the data frame was not found in the ACL.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDisassociateIndication(uint64_t DeviceAddress,
                                                    uint8_t DisassociateReason,
                                                    uint16_t SecurityUse,
                                                    uint8_t ACLEntry)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacDisassociateConfirm
 *!< @brief                  : Reports the results of an MLME-DISASSOCIATE.request primitive.
 *!< Parameters              :
 *!<                   Input : status - The status of the disassociation attempt.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacDisassociateConfirm(GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacBeaconNotifyIndication
 *!< @brief                  : Used to send parameters contained within a beacon frame received by the MAC sublayer
 *!<                         : to the next higher layer. The primitive also sends a measure of the LQ and the time
 *!<                         : the beacon frame was received.
 *!< Parameters              :
 *!<                   Input : BSN - The beacon sequence number.
 *!<                         : PANDescriptor - The PANDescriptor for the received beacon.
 *!<                         : PendAddrSpec - The beacon pending address specification.
 *!<                         : AddrList - The list of addresses of the devices for which the beacon source has data.
 *!<                         : sduLength - The number of octets contained in the beacon payload of the beacon frame
 *!<                         :             received by the MAC sublayer.
 *!<                         : sdu - The set of octets comprising the beacon payload to be transferred from the MAC
 *!<                         :       sublayer entity to the next higher layer.
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacBeaconNotifyIndication(uint8_t BSN,
                                                    IEEE_802_15_4_PANDescr_t* PANDescriptor,
                                                    uint8_t PendAddrSpec,
                                                    uint64_t* AddrList,
                                                    uint8_t sduLength,
                                                    uint8_t *sdu)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGetRequest
 *!< @brief                  : Requests information about a given PIB attribute.
 *!< Parameters              :
 *!<                   Input : PIBAttribute - The identifier of the PIB attribute to read.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacGetRequest(uint8_t PIBAttribute)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGetConfirm
 *!< @brief                  : Requests information about a given PIB attribute.
 *!< Parameters              :
 *!<                   Input : status - The result of the request for MAC PIB attribute information.
 *!<                         : PIBAttribute - The identifier of the MAC PIB attribute that was read.
 *!<                         :
 *!<                   Output: PIBAttributeValue - The value of the indicated MAC PIB attribute that was read.
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacGetConfirm(IEEE_802_15_4_PHY_Enum_t status,
                                        uint8_t PIBAttribute,
                                        void* PIBAttributeValue)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGtsRequest
 *!< @brief                  : Allows a device to send a request to the PAN coordinator to allocate
 *!<                         : a new GTS or to deallocate an existing GTS.
 *!< Parameters              :
 *!<                   Input : GTSCharacteristics - The characteristics of the GTS request.
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacGtsRequest(IEEE_802_15_4_GTSCaracter_t GTSCharacteristics,
                                        bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGtsConfirm
 *!< @brief                  : Reports the results of a request to allocate a new GTS or deallocate an
 *!<                         : existing GTS.
 *!< Parameters              :
 *!<                   Input : GTSCharacteristics - The characteristics of the GTS.
 *!<                         : status - The status of the GTS request.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacGtsConfirm(IEEE_802_15_4_GTSCaracter_t GTSCharacteristics,
                                        GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGtsIndication
 *!< @brief                  : Indicates that a GTS has been allocated or that a previously allocated
 *!<                         : GTS has been deallocated.
 *!< Parameters              :
 *!<                   Input : DevAddress - The short address of the device that has been allocated or deallocated a GTS.
 *!<                         : GTSCharacteristics - The characteristics of the GTS.
 *!<                         : SecurityUse - An indication of whether the received frame is using security. This value
 *!<                         :               is set to TRUE if the security enable subfield was set to 1 or FALSE
 *!<                         :               if the security enabled subfield was was set to 0.
 *!<                         : ACLEntry - The macSecurityMode parameter value from the ACL entry associated with the sender
 *!<                         :            of the data frame. This value is set to 0x08 if frame. This value is set to 0x08
 *!<                         :            if the sender of the data frame was not found in the ACL.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacGtsIndication(uint16_t DevAddress,
                                           IEEE_802_15_4_GTSCaracter_t GTSCharacteristics,
                                           bool SecurityUse,
                                           uint8_t ACLEntry)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacGtsIndication
 *!< @brief                  : Indicates that a GTS has been allocated or that a previously allocated
 *!<                         : GTS has been deallocated.
 *!< Parameters              :
 *!<                   Input : OrphanAddress - The address of the orphaned device.
 *!<                         : SecurityUse - An indication of whether the received frame is using security. This value
 *!<                         :               is set to TRUE if the security enable subfield was set to 1 or FALSE
 *!<                         :               if the security enabled subfield was was set to 0.
 *!<                         : ACLEntry - The macSecurityMode parameter value from the ACL entry associated with the sender
 *!<                         :            of the data frame. This value is set to 0x08 if frame. This value is set to 0x08
 *!<                         :            if the sender of the data frame was not found in the ACL.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacOrphanIndication(uint64_t OrphanAddress,
                                              bool SecurityUse,
                                              uint8_t ACLEntry)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacOrphanResponse
 *!< @brief                  : Indicates that a GTS has been allocated or that a previously allocated
 *!<                         : GTS has been deallocated.
 *!< Parameters              :
 *!<                   Input : OrphanAddress - The address of the orphaned device.
 *!<                         : ShortAddress - The short address allocated to the orphaned device if it is associated with
 *!<                         :                this coordinator. The special short address 0xfffe indicates that no short address was
 *!<                         :                allocated, and the device will use its 64 bit extended address in all communications. If
 *!<                         :                the device was not associated with this coordinator, this field will contain the value
 *!<                         :                0xffff and be ignored on receipt.
 *!<                         : AssociatedMember - TRUE if the orphaned device is associated with this coordinator or FALSE otherwise.
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacOrphanResponse(uint64_t OrphanAddress,
                                            uint16_t ShortAddress,
                                            bool AssociatedMember,
                                            bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacResetRequest
 *!< @brief                  : Allows the next higher layer to request that the MLME performs a reset operation.
 *!< Parameters              :
 *!<                   Input : SetDefaultPIB - If TRUE, the MAC sublayer is reset and all MAC PIB attributes are set to their default values.
 *!<                         :                 If FALSE, the MAC sublayer is reset but all MAC PIB attributes retain their values prior to the
 *!<                         :                 generation of the MLME-RESET.request primitive.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacResetRequest(bool SetDefaultPIB)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacResetConfirm
 *!< @brief                  : Reports the results of the reset operation.
 *!< Parameters              :
 *!<                   Input : status - The result of the reset operation.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacResetConfirm(GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacRxEnableRequest
 *!< @brief                  : Allows the next higher layer to request that the receiver is enable for a finite period of time.
 *!< Parameters              :
 *!<                   Input : DeferPermit - TRUE if the receiver enable can be deferred until during the next superframe
 *!<                         :               if the requested time has already passed. FALSE if the receiver enable is only
 *!<                         :               to be attempted in the current superframe. This parameter is ignored for
 *!<                         :               nonbeacon-enabled PANs.
 *!<                         : RxOnTime - The number of symbols from the start of the  superframe before the receiver
 *!<                         :            superframe before the receiver is to be enabled. The precision of this value is
 *!<                         :            a minimum of 20 bits, with the lowest 4 bits being the least significant. This
 *!<                         :            parameter is ignored for nonbeacon-enabled PANs.
 *!<                         : RxOnDuration - The number of symbols for which the receiver is to be enabled.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacRxEnableRequest(GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacRxEnableConfirm
 *!< @brief                  : Reports the results of the attempt to enable the receiver.
 *!< Parameters              :
 *!<                   Input : status - The result of the reset operation.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacRxEnableConfirm(bool DeferPermit,
                                             uint16_t RxOnTime,
                                             uint16_t RxOnDuration)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacScanRequest
 *!< @brief                  : Used to initiate a channel scan over a given list of channels. A device can use a channel scan to
 *!<                         : measure the energy on the channel, search for the coordinator with which it associated, or search
 *!<                         : for all coordinatorstransmitting beacon frames within the POS of the scanning device.
 *!< Parameters              :
 *!<                   Input : ScanType - Indicates the type of scan performed:
 *!<                         :                 0 x 00 = ED scan (FFD only).
 *!<                         :                 0 x 01 = active scan (FFD only).
 *!<                         :                 0 x 02 = passive scan.
 *!<                         :                 0 x 03 = orphan scan.
 *!<                         : ScanChannels - The 5 MSBs (b27 , ... , b31 ) are reserved. The 27 LSBs (b0 , b1 , ... b26 ) indicate which
 *!<                         :                channels are to be scanned (1 = scan, 0 = do not scan) for each of the 27 valid channels
 *!<                         : ScanDuration - A value used to calculate the length of time to spend scanning each channel for ED, active,
 *!<                         :                and passive scans. This parameter is ignored for orphan scans.
 *!<                         :                The time spent scanning each channel is [aBaseSuperframeDuration * (2^n + 1)]
 *!<                         :                symbols, where n is the value of the ScanDuration parameter.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacScanRequest(uint8_t ScanType,
                                         uint32_t ScanChannels,
                                         uint8_t ScanDuration)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacScanConfirm
 *!< @brief                  : Reports the result of the channel scan request.
 *!< Parameters              :
 *!<                   Input : status - The status of the scan request.
 *!<                         : ScanType - Indicates the type of scan performed:
 *!<                         :                 0 x 00 = ED scan (FFD only).
 *!<                         :                 0 x 01 = active scan (FFD only).
 *!<                         :                 0 x 02 = passive scan.
 *!<                         :                 0 x 03 = orphan scan.
 *!<                         : UnscannedChannels - Indicate which channels are to be scanned (1 = not scaned, 0 = scaned or not requested).
 *!<                         :                      This parameter is only valid for passive or active scans.
 *!<                         : ResultListSize - The number of elements returned in the appropriate result lists. This value is 0
 *!<                         :                  for the result of an orphan scan.
 *!<                         : EnergyDetectList - The list of energy measurements, one for each channel searched during an ED scan.
 *!<                         :                    This parameter is null for active, passive, and orphan scans.
 *!<                         : PANDescriptorList - The list of PAN descriptors, one for each beacon found during an active or passive scan.
 *!<                         :                     This parameter is null for ED and orphan scans.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacScanConfirm(GeneralMacRequestStatusType status,
                                         uint8_t ScanType,
                                         uint32_t UnscannedChannels,
                                         uint16_t ResultListSize,
                                         uint8_t* EnergyDetectList,
                                         IEEE_802_15_4_PANDescr_t* PANDescriptorList)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacComStatusIndication
 *!< @brief                  : Allows the MLME to indicate a communications status.
 *!< Parameters              :
 *!<                   Input : PANId - The 16 bit PAN identifier of the device from which the frame was received or to
 *!<                         :         which the frame was being sent.
 *!<                         : SrcAddrMode - The source addressing mode for this primitive. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : SrcAddr - The individual device address of the entity from which the frame causing the
 *!<                         :           error originated.
 *!<                         : DstAddrMode - The destination addressing mode for this primitive. This value can take one of the
 *!<                         :               following values:
 *!<                         :                  0 x 00 = no address (addressing fields omitted).
 *!<                         :                  0 x 01 = reserved.
 *!<                         :                  0 x 02 = 16 bit short address.
 *!<                         :                  0 x 03 = 64 bit extended address.
 *!<                         : DstAddr - The individual device address of the device for which the frame was intended.
 *!<                         : status - The communications status.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacComStatusIndication(uint16_t PANId,
                                                 uint8_t SrcAddrMode,
                                                 uint64_t SrcAddr,
                                                 uint8_t DstAddrMode,
                                                 uint64_t DstAddr,
                                                 GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacSetRequest
 *!< @brief                  : Attempts to write the given value to the indicated MAC PIB attribute.
 *!< Parameters              :
 *!<                   Input : PIBAttribute - The identifier of the MAC PIB attribute to write.
 *!<                         : PIBAttributeValue - The value to write to the indicated MAC PIB attribute.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacSetRequest(uint8_t PIBAttribute,
                                        void* PIBAttributeValue)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacSetConfirm
 *!< @brief                  : Reports the results of an attempt to write a value to a MAC PIB attribute.
 *!< Parameters              :
 *!<                   Input : status - The result of the request to write the MAC PIB attribute.
 *!<                         : PIBAttribute - The identifier of the MAC PIB attribute that was written.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacSetConfirm(GeneralMacRequestStatusType status,
                                        uint8_t PIBAttribute)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacStartRequest
 *!< @brief                  : Makes a request for the device to start using a new superframe configuration.
 *!< Parameters              :
 *!<                   Input : PANId - The PAN identifier to be used by the beacon.
 *!<                         : LogicalChannel - The logical channel on which to start transmitting beacons.
 *!<                         : BeaconOrder - How often the beacon is to be transmitted. The beacon order, BO,
 *!<                         :               and  the beacon interval, BI, are related as follows: for 0 ≤ BO ≤ 14,
 *!<                         :               BI = aBaseSuperframeDuration * 2^BO symbols. If BO = 15, the coordinator
 *!<                         :               will not transmit a beacon, and the SuperframeOrder parameter value is ignored.
 *!<                         : SuperframeOrder -  The length of the active portion of the superframe, including the beacon frame.
 *!<                         :                    The superframe order, SO, and the superframe duration, SD, are related as follows:
 *!<                         :                    for 0 ≤ SO ≤ BO ≤ 14, SD = aBaseSuperframeDuration * 2^SO symbols. If SO = 15,
 *!<                         :                    the superframe will not be active after thebeacon.
 *!<                         : PANCoordinator - If this value is TRUE, the device will become the PAN coordinator of a new PAN. If
 *!<                         :                  this value is FALSE, thedevice will begin transmitting beacons on the PAN with which
 *!<                         :                  it isassociated.
 *!<                         : BatteryLifeExtension - If this value is TRUE, the receiver of the beaconing device is disabled macBattLifeExtPeriods
 *!<                         :                        full backoff periods after the interframe spacing (IFS) period of the beacon frame. If this
 *!<                         :                        value is FALSE, the receiver of the beaconing device remains enabled for the entire CAP.
 *!<                         : CoordRealignment - TRUE if a coordinator realignment command is to be transmitted prior to changing the superframe
 *!<                         :                    configuration or FALSE otherwise.
 *!<                         : SecurityEnable - TRUE if security is enabled for beacon transmissions or FALSE otherwise.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacStartRequest(uint16_t PANId,
                                          uint8_t LogicalChannel,
                                          uint8_t BeaconOrder,
                                          uint8_t SuperframeOrder,
                                          bool PANCoordinator,
                                          bool BatteryLifeExtension,
                                          bool CoordRealignment,
                                          bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacStartConfirm
 *!< @brief                  : Reports the results of the attempt to start using a new superframe configuration.
 *!< Parameters              :
 *!<                   Input : status - The result of the attempt to start using an updated superframe configuration.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacStartConfirm(GeneralMacRequestStatusType status)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacSyncRequest
 *!< @brief                  : Requests to synchronize with the coordinator by acquiring and, if
 *!<                         : specified, tracking its beacons.
 *!< Parameters              :
 *!<                   Input : LogicalChannel - The logical channel on which to attempt coordinator synchronization.
 *!<                         : TrackBeacon - TRUE if the MLME is to synchronize with the next beacon and attempt to
 *!<                         :               track all future beacons. FALSE if the MLME is to synchronize with only
 *!<                         :               the next beacon.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacSyncRequest(uint8_t LogicalChannel,
                                         bool TrackBeacon)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacSyncLossIndication
 *!< @brief                  :
 *!< Parameters              :
 *!<                   Input : LossReason - The reason that synchronization was lost.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacSyncLossIndication(GeneralMacRequestStatusType LossReason)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacPollRequest
 *!< @brief                  : Prompts the device to request data from the coordinator.
 *!< Parameters              :
 *!<                   Input : CoordAddrMode - The addressing mode of the coordinator to which the poll
 *!<                         :                 is intended. This parameter can take one of the following values:
 *!<                         :                          2 = 16 bit short address,
 *!<                         :                          3 = 64 bit extended address.
 *!<                         : CoordPANId - The PAN identifier of the coordinator to which the poll is intended.
 *!<                         : CoordAddress - The address of the coordinator to which the poll is intended.
 *!<                         : SecurityEnable - TRUE if security is enabled for this transfer or FALSE otherwise.
 *!<                         :
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacPollRequest(uint8_t CoordAddrMode,
                                         uint8_t CoordPANId,
                                         uint64_t CoordAddress,
                                         bool SecurityEnable)
{

}


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_MacPollConfirm
 *!< @brief                  : Reports the results of a request to poll the coordinator for data.
 *!< Parameters              :
 *!<                   Input : status - The status of the data request.
 *!<                         :
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
static void IEEE_802_15_4_MacPollConfirm(GeneralMacRequestStatusType status)
{

}
