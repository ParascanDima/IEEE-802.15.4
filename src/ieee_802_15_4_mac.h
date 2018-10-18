/*
 * ieee_802_15_4_mac.h
 *
 *  Created on: 8 окт. 2018 г.
 *      Author: Dumitru
 */

#ifndef _IEEE_802_15_4_MAC_H_
#define _IEEE_802_15_4_MAC_H_


/***************************Imports*************************************************/

#include "stdint.h"
#include "ieee_802_15_4_cnf.h"

/***************************Public Macro Definitions********************************/

/*!<
 *!< @brief Definition of fields size per Figure 34 IEEE Std 802.15.4-2003 (Figure 35 IEEE Std 802.15.4-2003)
 *!< */
#define IEEE_802_15_4_FRAME_LENGTH_BYTES                (1)
#define IEEE_802_15_4_FRAME_CONTROL_BYTES               (2)
#define IEEE_802_15_4_SEQUENCE_NUMBER_BYTES             (1)
#define IEEE_802_15_4_MAX_DST_PANID_BYTES               (2)
#define IEEE_802_15_4_MAX_DST_ADDR_BYTES                (8)
#define IEEE_802_15_4_MAX_SRC_PANID_BYTES               (2)
#define IEEE_802_15_4_MAX_SRC_ADDR_BYTES                (8)
#define IEEE_802_15_4_FCS_BYTES                         (2)

#ifdef IEEE_802_15_4_2011_COMPLIANT
#define IEEE_802_15_4_MAX_AUX_SECURITY_HEADER_BYTES     (8 + 4 + 2)
#endif

/*!<
 *!< @brief note these have the same values as Frame Types in Table 65 of IEEE Std 802.15.4-2003 or Table 2 of IEEE Std 802.15.4-2011
 *!< */
#define IEEE_802_15_4_FRAME_TYPE_BEACON                 (0x00)
#define IEEE_802_15_4_FRAME_TYPE_DATA                   (0x01)
#define IEEE_802_15_4_FRAME_TYPE_ACK                    (0x02)
#define IEEE_802_15_4_FRAME_TYPE_MAC_COMMAND            (0x03)



/*!<
 *!< @brief note these have the same values as Frame Types in Table 65 of IEEE Std 802.15.4-2003 or Table 2 of IEEE Std 802.15.4-2011
 *!< */
#define IEEE_802_15_4_MAC_CMD_ASSOCIATION_REQUEST           (0x01)
#define IEEE_802_15_4_MAC_CMD_ASSOCIATION_RESPONSE          (0x02)
#define IEEE_802_15_4_MAC_CMD_DIASSOCIATION_NOTIFICATION    (0x03)
#define IEEE_802_15_4_MAC_CMD_DATA_REQUEST                  (0x04)
#define IEEE_802_15_4_MAC_CMD_PANID_CONFLICT_NOTIFICATION   (0x05)
#define IEEE_802_15_4_MAC_CMD_ORPHAN_NOTIFICATION           (0x06)
#define IEEE_802_15_4_MAC_CMD_BEACON_REQUEST                (0x07)
#define IEEE_802_15_4_MAC_CMD_COORDINATOR_REALIGNMENT       (0x08)
#define IEEE_802_15_4_MAC_CMD_GTS_REQUEST                   (0x09)



/*!<
 *!< @brief source and destination addressing modes, IEEE Std 802.15.4-2011 Table 3 (IEEE Std 802.15.4-2003 Table 66)
 *!< */
#define IEEE_802_15_4_NO_ADDR_FIELD                     (0) // 00 PAN identifier and address fields are not present.
//#define RESERVED_FIELD                                (1)    01 Reserved.
#define IEEE_802_15_4_SHORT_ADDR_FIELD                  (2) // 10 Address field contains a 16-bit short address.
#define IEEE_802_15_4_LONG_ADDR_FIELD                   (3) // 11 Address field contains a 64-bit extended address.


#ifdef IEEE_802_15_4_2011_COMPLIANT
/*!<
 *!< @brief Security levels described in Table 58 of IEEE Std 802.15.4-2011
 *!< */
#define IEEE_802_15_4_NO_SECURITY_LEVEL                 (0x00)   /* No security */
#define IEEE_802_15_4_SECURITY_LEVEL_1                  (0x01)   /* MIC-32 */
#define IEEE_802_15_4_SECURITY_LEVEL_2                  (0x02)   /* MIC-64 */
#define IEEE_802_15_4_SECURITY_LEVEL_3                  (0x03)   /* MIC-128 */
#define IEEE_802_15_4_SECURITY_LEVEL_4                  (0x04)   /* ENC */
#define IEEE_802_15_4_SECURITY_LEVEL_5                  (0x05)   /* ENC-MIC-32 */
#define IEEE_802_15_4_SECURITY_LEVEL_6                  (0x06)   /* ENC-MIC-64 */
#define IEEE_802_15_4_SECURITY_LEVEL_7                  (0x07)   /* ENC-MIC-128 */

/*!<
 *!< @brief Key ID described in Table 59 of IEEE 802.15.4-2011
 *!< */
#define IEEE_802_15_4_KEY_ID_MODE_IMPLICIT              (0x00)
#define IEEE_802_15_4_KEY_ID_MODE_1BYTE                 (0x01)
#define IEEE_802_15_4_KEY_ID_MODE_4BYTE                 (0x02)
#define IEEE_802_15_4_KEY_ID_MODE_8BYTE                 (0x03)
#endif



/***************************Public Type Definitions********************************/

/*!<
 *!< @brief Frame control type (per IEEE Std 802.15.4-2003 Figure 35 )
 *!<
 *!<  Information defining the frame type, addressing fields, and other control flags.
 *!<
 *!< */
typedef union {
    uint16_t    value;
    struct {
        unsigned    frameType       :3;       /* Frame type indicator. Shall be set to one of the nonreserved values listed in Table 65 IEEE Std 802.15.4-2003  */
        unsigned    securityEnabled :1;       /* Shows either frame is cryptographically protected by the MAC sublayer or not                                   */
        unsigned    framePending    :1;       /* Shows if more then one frame should be send to recepient with response on one request                          */
        unsigned    ackRequest      :1;       /* Acknowledge from recepient is required                                                                         */
        unsigned    intraPAN        :1;       /* Specifies whether the MAC frame is to be sent within the same PAN (intra-PAN) or to another PAN (inter-PAN)    */
        unsigned    /*reserved*/    :3;       /* reserved by IEEE Std 802.15.4-2003                                                                             */
        unsigned    dstAddrMode     :2;       /* Specifies the destination addressing type (short addressing or long addressing)                                */
#ifdef IEEE_802_15_4_2011_COMPLIANT
        unsigned    frameVersion    :2;       /* The Frame Version field specifies the version number corresponding to the frame                                */
#else
        unsigned    /*reserved*/    :2;       /* reserved by IEEE Std 802.15.4-2003                                                                             */
#endif
        unsigned    srcAddrMode     :2;       /* Specifies the source addressing type (short addressing or long addressing)                                     */
    }Field;
}IEEE_802_15_4_FrameCtrl;


#ifdef IEEE_802_15_4_2011_COMPLIANT
/*!<
 *!< @brief Security control type (per IEEE Std 802.15.4-2011 Figure 63)
 *!< */
typedef union{
    uint8_t value;
    struct{
        unsigned       securityLevel    :3;
        unsigned       keyIdMode        :2;
        unsigned       /*reserved*/     :3;
    }Field;
}IEEE_802_15_4_SecurityCtrl;


/*!<
 *!< @brief Key ID type (per IEEE Std 802.15.4-2011 Figure 64)
 *!< */
typedef struct{
    uint64_t keySource;
    uint8_t keyIndex;
}IEEE_802_15_4_KeyIdType;


/*!<
 *!< @brief Format of the auxiliary security header type (per IEEE Std 802.15.4-2011 Figure 62)
 *!< */
typedef struct{

    /* 1 octet */
    IEEE_802_15_4_SecurityCtrl securityControl;

    /* 4 octets */
    uint32_t frameCounter;

    /* 9 octets maximum */
    IEEE_802_15_4_KeyIdType keyID;

}IEEE_802_15_4_AuxSecurityHeaderType;
#endif


/*!<
 *!< @brief Mac Beacon superframe specification description (per IEEE Std 802.15.4-2003 Figure 40 )
 *!< */
typedef struct
{
    uint8_t beaconOrder       :4;
    uint8_t superFrameOrder   :4;
    uint8_t finalCAPslot      :4;
    uint8_t battLifeExt       :1;
    uint8_t /* reserved */    :1;
    uint8_t panCoordinator    :1;
    uint8_t assocPermit       :1;

}IEEE_802_15_4_BeaconSuperFrameSpec_t;


/*!<
 *!< @brief Mac GTS specification description (per IEEE Std 802.15.4-2003 Figure 41 )
 *!< */
typedef struct
{
    uint8_t gtsDescriptionCnt :3;
    uint8_t /* reserved */    :1;
    uint8_t panCoordinator    :1;

}IEEE_802_15_4_GTSSpec_t;


/*!<
 *!< @brief Mac GTS direction description (per IEEE Std 802.15.4-2003 Figure 42 )
 *!< */
typedef struct
{
    uint8_t gtsDirMask        :7;
    uint8_t /* reserved */    :1;

}IEEE_802_15_4_GTSDir_t;


/*!<
 *!< @brief Mac GTS direction description (per IEEE Std 802.15.4-2003 Figure 43 )
 *!< */
typedef struct
{
    uint16_t devShrtAddr      :16;
    uint8_t  gtsStartSlot     :4;
    uint8_t  gtsLength        :4;

}IEEE_802_15_4_GTSDescriptor_t;


/*!<
 *!< @brief Mac GTS description (per IEEE Std 802.15.4-2003 Figure 38 )
 *!< */
typedef struct
{
    IEEE_802_15_4_GTSSpec_t         gtsSpec;
    IEEE_802_15_4_GTSDir_t          gtsDirection;
    IEEE_802_15_4_GTSDescriptor_t*  gtsList;

}IEEE_802_15_4_GTS_t;



/*!<
 *!< @brief Mac Pending address specification description (per IEEE Std 802.15.4-2003 Figure 44 )
 *!< */
typedef struct
{
    uint8_t nrShortPendAddrInList     :3;
    uint8_t /* reserved */            :1;
    uint8_t nrLongPendAddrInList      :3;
    uint8_t /* reserved */            :1;

}IEEE_802_15_4_PendAddrSpec_t;


/*!<
 *!< @brief Mac Address list field description
 *!< */
typedef uint64_t   IEEE_802_15_4_PendAddrList_t[7];


/*!<
 *!< @brief Mac Command frame identifier description
 *!< */
typedef uint8_t   IEEE_802_15_4_CmdId_t;



/*!<
 *!< @brief Mac GTS description (per IEEE Std 802.15.4-2003 Figure 38 )
 *!< */
typedef struct
{
    IEEE_802_15_4_PendAddrSpec_t pendAddrSpec;
    IEEE_802_15_4_PendAddrList_t pendAddrList;

}IEEE_802_15_4_PendAddr_t;


/*!<
 *!< @brief MAC Beacon frame format (per IEEE Std 802.15.4-2003 Figure 37 )
 *!< */
typedef struct
{
    /* ========================= MHR ========================= */
    IEEE_802_15_4_FrameCtrl   frameControl;
    uint8_t     sequenceNumber;            /* packet sequence number */
    uint16_t    srcPANID;
    uint64_t    srcAddr;

    /* ===================== MAC PAYLOAD ===================== */
    IEEE_802_15_4_BeaconSuperFrameSpec_t superFrameSpec;
    IEEE_802_15_4_GTS_t                  gtsFields;
    IEEE_802_15_4_PendAddr_t                 pendingAddr;

    /* ========================= MFR ========================= */
    uint16_t    fcs;

}IEEE_802_15_4_MacBeaconFrame_t;



/*!<
 *!< @brief MAC Data frame format (per IEEE Std 802.15.4-2003 Figure 45 )
 *!< */
typedef struct
{

    /* ========================= MHR ========================= */
    IEEE_802_15_4_FrameCtrl   frameControl;
    uint8_t     sequenceNumber;
    uint16_t    dstPANID;
    uint16_t    srcPANID;
    uint64_t    dstAddr;
    uint64_t    srcAddr;

    /* ===================== MAC PAYLOAD ===================== */
    uint8_t *   payload;

    /* ========================= MFR ========================= */
    uint16_t    fcs;

#ifdef IEEE_802_15_4_2011_COMPLIANT
    IEEE_802_15_4_AuxSecurityHeaderType auxSecurityHeader;
#endif

} IEEE_802_15_4_MacDataFrame_t;



/*!<
 *!< @brief MAC Acknowledge frame format (per IEEE Std 802.15.4-2003 Figure 46 )
 *!< */
typedef struct
{
    /* ========================= MHR ========================= */
    IEEE_802_15_4_FrameCtrl   frameControl;
    uint8_t     sequenceNumber;

    /* ========================= MFR ========================= */
    uint16_t    fcs;

} IEEE_802_15_4_MacAcknowledgeFrame_t;



/*!<
 *!< @brief MAC command frame format (per IEEE Std 802.15.4-2003 Figure 47 )
 *!< */
typedef struct
{

    /* ========================= MHR ========================= */
    IEEE_802_15_4_FrameCtrl   frameControl;
    uint8_t     sequenceNumber;
    uint16_t    dstPANID;
    uint16_t    srcPANID;
    uint64_t    dstAddr;
    uint64_t    srcAddr;

    /* ===================== MAC PAYLOAD ===================== */
    IEEE_802_15_4_CmdId_t cmdId;
    uint8_t *  cmdPayload;

    /* ========================= MFR ========================= */
    uint16_t    fcs;

} IEEE_802_15_4_MacCommandFrame_t;

/***************************Public Data Definitions********************************/




/***************************Public Function Definitions****************************/


#endif /* _IEEE_802_15_4_MAC_H_ */
