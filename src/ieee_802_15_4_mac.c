/*
 * ieee_802_15_4_mac.c
 *
 *  Created on: 8 ���. 2018 �.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ieee_802_15_4_mac.h"

/**************Private Macro Definitions*****************/



/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/

/*!<
 *!< @brief MAC constants
 *!< */
const uint8_t aBaseSlotDuration = (uint8_t)60;
const uint16_t aBaseSuperframeDuration = (uint16_t)960;
const uint8_t aMaxBE = (uint8_t)5;
const uint8_t aMaxBeaconOverhead = (uint8_t)75;
const uint8_t aMaxBeaconPayloadLength = (uint8_t)52;
const uint8_t aGTSDescPersistenceTime = (uint8_t)4;
const uint8_t aMaxFrameOverhead = (uint8_t)25;
const uint16_t aMaxFrameResponseTime = (uint16_t)1220;
const uint8_t aMaxFrameRetries = (uint8_t)3;
const uint8_t aMaxLostBeacons = (uint8_t)4;
const uint8_t aMaxMACFrameSize = (uint8_t)102;
const uint8_t aMaxSIFSFrameSize = (uint8_t)18;
const uint16_t aMinCAPLength = (uint16_t)440;
const uint8_t aMinLIFSPeriod = (uint8_t)40;
const uint8_t aMinSIFSPeriod = (uint8_t)12;
const uint8_t aNumSuperframeSlots = (uint8_t)16;
const uint16_t aResponseWaitTime = (uint16_t)30720;
const uint8_t aUnitBackoffPeriod = (uint8_t)20;

/*!<
 *!< @brief Constants that are not defined by the IEEE Std 802.15.4-2003 but are upper side of threshold
 *!< */
const uint8_t ACLSecurityMaterialLengthMax = (uint8_t)26;
const uint8_t macACLEntryDescriptorSetSizeMax = UINT8_MAX;

/*!<
 *!< @brief MAC PIB attributes
 *!< */
uint8_t macAckWaitDuration = (uint8_t)54;
bool macAssociationPermit = (bool)false;
bool macAutoRequest = (bool)true;
bool macBattLifeExt = (bool)false;
uint8_t macBattLifeExtPeriods = (uint8_t)6;
uint8_t *macBeaconPayload = NULL;
uint8_t macBeaconPayloadLength = (uint8_t)0;
uint8_t macBeaconOrder = (uint8_t)15;
uint32_t macBeaconTxTime = (uint32_t)0x00;
uint8_t macBSN;
uint64_t macCoordExtendedAddress;
uint16_t macCoordShortAddress = (uint16_t)0xFFFF;
uint8_t macDSN;
bool macGTSPermit = (bool)true;
uint8_t macMaxCSMABackoffs = 4;
uint8_t macMinBE = 3;
uint16_t macPANId = (uint16_t)0xFFFF;
bool macPromiscuousMode = (bool)false;
bool macRxOnWhenIdle = (bool)false;
uint16_t macShortAddress = (uint16_t)0xFFFF;
uint8_t macSuperframeOrder = (uint8_t)15;
uint16_t macTransactionPersistenceTime = (uint16_t)0x01F4;

/*!<
 *!< @brief MAC PIB security attributes
 *!< */
uint8_t macACLEntryDescriptorSet[] = "";
uint8_t macACLEntryDescriptorSetSize = (uint8_t)0x00;
bool macDefaultSecurity = (bool)false;
uint8_t macDefaultSecurityMaterialLength = (uint8_t)0x15;
uint8_t macDefaultSecurityMaterial[ACLSecurityMaterialLengthMax] = "";
uint8_t macDefaultSecuritySuite = (uint8_t)0x00;
uint8_t macSecurityMode = (uint8_t)0x00;

/*!<
 *!< @brief Elements of ACL entry descriptor
 *!< */
uint64_t ACLExtendedAddress;
uint16_t ACLShortAddress = (uint16_t)0xFFFF;
uint16_t ACLPANId;
uint8_t ACLSecurityMaterialLength = (uint8_t)21;
uint8_t ACLSecurityMaterial[ACLSecurityMaterialLengthMax] = "";
uint8_t ACLSecuritySuite = (uint8_t)0x00;
/**************Public Variable Definitions***************/

const uint64_t aExtendedAddress __attribute__((weak)) = (uint64_t)0xFFFFFFFFFFFFFFFF;

/**************Private Function Definitions**************/





/**************Public Function Definitions***************/

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
                                         LastMsduTransmissionStatusType status)
{

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
                                          PurgedMsduRequestStatusType status)
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
                                               AssociationStatusType status,
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
                                               AssociationStatusType status)
{

}


/****************************************************************************************
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
static void IEEE_802_15_4_MacDisassociateConfirm(AssociationStatusType status)
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
