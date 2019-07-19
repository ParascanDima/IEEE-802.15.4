/*
 * ieee_802_15_4_phy.h
 *
 *  Created on: 8 окт. 2018 г.
 *      Author: Dumitru
 */

#ifndef _IEEE_802_15_4_PHY_H_
#define _IEEE_802_15_4_PHY_H_


/***************************Imports*************************************************/

#include "stdint.h"
#include "stdbool.h"
#include "ieee_802_15_4_cnf.h"

/***************************Public Macro Definitions********************************/

#ifdef IEEE_802_15_4_2011_COMPLIANT
/*!<
 *!< @brief PHY PIB attributes number (per IEEE Std 802.15.4-2011 Table 71)
 *!< */
#define PHY_ATTRIB_NUMBER                               36

#else
/*!<
 *!< @brief PHY PIB attributes number (per IEEE Std 802.15.4-2003 Table 18)
 *!< */
#define PHY_ATTRIB_NUMBER                               4

#endif

/***************************Public Type Definitions********************************/

#ifdef IEEE_802_15_4_2011_COMPLIANT
/*!<
 *!< @brief PHY Channel description (per IEEE Std 802.15.4-2011 Table 71 )
 *!< */
typedef struct
{

}IEEE_802_15_4_PhyChnDescr;

#else

/*!<
 *!< @brief PHY Channel description (per IEEE Std 802.15.4-2003 Table 19 )
 *!< */
typedef union
{
    uint32_t value;
    struct{
        uint8_t    /*  reserved  */  : 5;
        uint8_t    ch27              : 1;
        uint8_t    ch26              : 1;
        uint8_t    ch25              : 1;
        uint8_t    ch24              : 1;
        uint8_t    ch23              : 1;
        uint8_t    ch22              : 1;
        uint8_t    ch21              : 1;
        uint8_t    ch20              : 1;
        uint8_t    ch19              : 1;
        uint8_t    ch18              : 1;
        uint8_t    ch17              : 1;
        uint8_t    ch16              : 1;
        uint8_t    ch15              : 1;
        uint8_t    ch14              : 1;
        uint8_t    ch13              : 1;
        uint8_t    ch12              : 1;
        uint8_t    ch11              : 1;
        uint8_t    ch10              : 1;
        uint8_t    ch9               : 1;
        uint8_t    ch8               : 1;
        uint8_t    ch7               : 1;
        uint8_t    ch6               : 1;
        uint8_t    ch5               : 1;
        uint8_t    ch4               : 1;
        uint8_t    ch3               : 1;
        uint8_t    ch2               : 1;
        uint8_t    ch1               : 1;
        uint8_t    ch0               : 1;
    }channel;
}IEEE_802_15_4_PhyChnDescr_t;

/*!<
 *!< @brief PHY transmit power description (per IEEE Std 802.15.4-2003 Table 19 )
 *!< */
typedef union
{
    uint8_t value;
    struct{
        int8_t   nomTxPow    : 6;
        uint8_t  tolerance   : 2;
    }bits;
}IEEE_802_15_4_PhyTxPower_t;

#endif

/*!<
 *!< @brief PHY PSDU format description (per IEEE Std 802.15.4-2003 Figure 16 )
 *!< */
typedef uint8_t* IEEE_802_15_4_PSDU_t;


/*!<
 *!< @brief PHY PPDU format description (per IEEE Std 802.15.4-2003 Figure 16 )
 *!< */
typedef struct{
    /* ========================= SHR ========================= */
    const uint32_t Preable;
    const uint8_t  SFD;

    /* ========================= PHR ========================= */
    uint8_t frameLength  :7;
    uint8_t /*reserved*/ :1;

    /* ===================== PHY PAYLOAD ===================== */
    IEEE_802_15_4_PSDU_t PSDU;

}IEEE_802_15_4_PPDU_t;


/*!<
 *!< @brief PHY enumeration description
 *!< */
typedef enum
{
    CHANNEL_BUSY = 0x00,         /* The CCA attempt has detected a busy channel.                                                   */
    BUSY_RX = 0x01,              /* The transceiver is asked to change its state while receiving.                                  */
    BUSY_TX = 0x02,              /* The transceiver is asked to change its state while transmitting.                               */
    FORCE_TRX_OFF = 0x03,        /* The transceiver is to be switched off.                                                         */
    IDLE = 0x04,                 /* The CCA attempt has detected an idle channel.                                                  */
    PHY_INVALID_PARAMETER = 0x05,/* A SET/GET request was issued with a parameter in the primitive that is out of the valid range. */
    RX_ON = 0x06,                /* The transceiver is in or is to be configured into the receiver enabled state.                  */
    PHY_SUCCESS = 0x07,          /* A SET/GET, an ED operation, or a transceiver state change was successful.                      */
    TRX_OFF = 0x08,              /* The transceiver is in or is to be configured into the transceiver disabled state.              */
    TX_ON = 0x09,                /* The transceiver is in or is to be configured into the transmitter enabled state.               */
    PHY_UNSUPPORTED_ATTRIBUTE = 0x0a /* A SET/GET request was issued with the identifier of an attribute that is not supported.        */
}IEEE_802_15_4_PHY_Enum_t;


/*!<
 *!< @brief IDs of PHY PIB attributes
 *!< */
typedef enum
{
#ifdef IEEE_802_15_4_2003_COMPLIANT
    PHY_CURRENT_CHANNEL = 0,
    PHY_CHANNELS_SUPPORTED,
    PHY_TRANSMIT_POWER,
    PHY_CCA_MODE,
#endif
}IEEE_802_15_4_PIB_ID_t;


/*!<
 *!< @brief
 *!< */
typedef enum
{
    enRequest = 0,
    enConfirm,
    enIndication
}IEEE_802_15_4_Service_t;


/*!<
 *!< @brief
 *!< */
typedef enum
{
    PD_DATA = 0,
    PLME_CCA,
    PLME_ED,
    PLME_GET,
    PLME_SET_TRX_STATE,
    PLME_SET
}IEEE_802_15_4_ServiceAccessPoint_t;


/*!<
 *!< @brief PD-Data.request type (per IEEE Std 802.15.4-2003   6.2.1 PHY data service (PD-DATA))
 *!< */
typedef void (*PD_Data_Request_t)(uint8_t psduLength, uint8_t* psdu);


/*!<
 *!< @brief PD-Data.confirm type (per IEEE Std 802.15.4-2003   6.2.1 PHY data service (PD-DATA))
 *!< */
typedef void (*PD_Data_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status);


/*!<
 *!< @brief PD-Data.indication type (per IEEE Std 802.15.4-2003   6.2.1 PHY data service (PD-DATA))
 *!< */
typedef void (*PD_Data_Indication_t)(uint8_t psduLength, uint8_t* psdu, uint8_t ppduLinkQuality);


/*!<
 *!< @brief PHY data service description (per IEEE Std 802.15.4-2003   6.2.1 PHY data service)
 *!< */
typedef struct
{
    /* Request is generated from MAC sublayer to transfer PSDU
     * psduLength - The number of octets contained in the PSDU to be transmitted by the PHY entity.
     * psdu       - The set of octets forming the PSDU to be transmitted by the PHY entity.
     *  */
    /* Valid psduLength range: <= aMaxPHYPacketSize */
    PD_Data_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range:  SUCCESS, RX_ON, TRX_OFF */
    PD_Data_Confirm_t Confirm;

    /* Indication transfers the received PSDU to MAC sublayer
     * */
    /* Valid psduLength range: <= aMaxPHYPacketSize */
    /* Valid psduLength range:  0x00 - 0xFF */
    PD_Data_Indication_t Indication;
}IEEE_802_15_4_PD_DATA_t;


/*!<
 *!< @brief PHY data service description (per IEEE Std 802.15.4-2003   6.2.1 PHY data service)
 *!< */
typedef struct
{

    /* PD-DATA primitive */
    IEEE_802_15_4_PD_DATA_t DATA;

}IEEE_802_15_4_PHY_SAP_t;


/*!<
 *!< @brief PLME-CCA.request type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-CCA))
 *!< */
typedef void (*PLME_CCA_Request_t)(void);


/*!<
 *!< @brief PLME-CCA.confirm type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-CCA))
 *!< */
typedef void (*PLME_CCA_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status);


/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-CCA))
 *!< */
typedef struct
{
    /* Request is generated  by the MLME and issued to its PLME whenever the CSMA-CA algorithm requires an assessment of the channel. */
    PLME_CCA_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range: TRX_OFF, TX_ON, BUSY, IDLE */
    PLME_CCA_Confirm_t Confirm;

}IEEE_802_15_4_PLME_CCA_t;


/*!<
 *!< @brief PLME-ED.request type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-ED))
 *!< */
typedef void (*PLME_ED_Request_t)(void);


/*!<
 *!< @brief PLME-ED.confirm type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-ED))
 *!< */
typedef void (*PLME_ED_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status, uint8_t* EnergyLevel);


/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-ED))
 *!< */
typedef struct
{
    /* Request is generated  by the MLME and issued to its PLME to request an ED measurement. */
    PLME_ED_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range: SUCCESS, TRX_OFF, TX_ON */
    /* Valid EnergyLevel range: 0x00 - 0xFF */
    PLME_ED_Confirm_t Confirm;

}IEEE_802_15_4_PLME_ED_t;


/*!<
 *!< @brief PLME-GET.request type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-GET))
 *!< */
typedef void (*PLME_GET_Request_t)(IEEE_802_15_4_PIB_ID_t PIBAttribID);


/*!<
 *!< @brief PLME-GET.confirm type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-GET))
 *!< */
typedef void (*PLME_GET_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue);



#ifndef IEEE_802_15_4_2011_COMPLIANT
/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-GET))
 *!< */
typedef struct
{
    /* Request is generated  by the MLME and issued to its PLME to obtain information from the PHY PIB. */
    /* Valid PIBAttribID range:  */
    PLME_GET_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range: SUCCESS, UNSUPPORTED_ATTRIBUTE */
    /* Valid PIBAttribID range:  */
    /* Valid PIBAttributeValue range: Attribute specific (see IEEE Std 802.15.4-2003 Table 19) */
    PLME_GET_Confirm_t Confirm;

}IEEE_802_15_4_PLME_GET_t;

#endif


/*!<
 *!< @brief PLME-SET-TRX-STATE.request type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET-TRX-STATE))
 *!< */
typedef void (*PLME_SET_TRX_STATE_Request_t)(IEEE_802_15_4_PHY_Enum_t state);


/*!<
 *!< @brief PLME-SET-TRX-STATE.confirm type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET-TRX-STATE))
 *!< */
typedef void (*PLME_SET_TRX_STATE_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status);

/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET-TRX-STATE))
 *!< */
typedef struct
{
    /* Request is generated  by the MLME and issued to its PLME when the current operational state of the receiver needs to be changed. */
    /* Valid state range:  RX_ON, TRX_OFF, FORCE_TRX_OFF, TX_ON */
    PLME_SET_TRX_STATE_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range:  SUCCESS, RX_ON, TRX_OFF, TX_ON, BUSY_RX, BUSY_TX */
    PLME_SET_TRX_STATE_Confirm_t Confirm;

}IEEE_802_15_4_PLME_SET_TRX_STATE_t;


#ifndef IEEE_802_15_4_2011_COMPLIANT

/*!<
 *!< @brief PLME-SET.request type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET))
 *!< */
typedef void (*PLME_SET_Request_t)(IEEE_802_15_4_PIB_ID_t PIBAttribID, uint8_t* PIBAttributeValue);


/*!<
 *!< @brief PLME-SET.confirm type (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET))
 *!< */
typedef void (*PLME_SET_Confirm_t)(IEEE_802_15_4_PHY_Enum_t status, IEEE_802_15_4_PIB_ID_t PIBAttribID);


/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service (PLME-SET))
 *!< */
typedef struct
{
    /* Request is generated  by the MLME and issued to its PLME to request an ED measurement. */
    /* Valid PIBAttribID range:  */
    /* Valid PIBAttributeValue range: Attribute specific (see IEEE Std 802.15.4-2003 Table 19) */
    PLME_SET_Request_t Request;

    /* Confirm shows the status of transmition */
    /* Valid status range:  SUCCESS, UNSUPPORTED_ATTRIBUTE, INVALID_PARAMETER */
    /* Valid PIBAttribID range:  */
    PLME_SET_Confirm_t Confirm;

}IEEE_802_15_4_PLME_SET_t;
#endif


/*!<
 *!< @brief PHY management service description (per IEEE Std 802.15.4-2003   6.2.2 PHY management service)
 *!< */
typedef struct
{
    /* PLME-CCA primitive */
    IEEE_802_15_4_PLME_CCA_t CCA;

    /* PLME-ED primitive */
    IEEE_802_15_4_PLME_ED_t ED;

#ifndef IEEE_802_15_4_2011_COMPLIANT

    /* PLME-GET primitive */
    IEEE_802_15_4_PLME_GET_t GET;

    /* PLME-SET primitive */
    IEEE_802_15_4_PLME_SET_t SET;

#endif /*IEEE_802_15_4_2011_COMPLIANT*/

    /* PLME-SET-TRX-STATE primitive */
    IEEE_802_15_4_PLME_SET_TRX_STATE_t SET_TRX_STATE;


}IEEE_802_15_4_PLME_SAP_t;


/*!<
 *!< @brief PHY data service description ( per IEEE Std 802.15.4-2003  )
 *!< */
typedef struct
{

    /* PD-DATA primitive */
    IEEE_802_15_4_PD_DATA_t DATA;

}IEEE_802_15_4_PD_SAP_t;


/*!<
 *!< @brief PHY main description ( per IEEE Std 802.15.4-2003  )
 *!< */
typedef struct
{

    /* Phy Data service */
    IEEE_802_15_4_PD_SAP_t PD_SAP;

    /*  PHY management service */
    IEEE_802_15_4_PLME_SAP_t PLME_SAP;

}IEEE_802_15_4_PHY_t;

/***************************Public Data Definitions********************************/

extern IEEE_802_15_4_PHY_t phyMain;


/***************************Public Function Definitions****************************/

/****************************************************************************************
 *!< Function                : IEEE_802_15_4_PhyInit
 *!< @brief                  : Initialization of PHY layer
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
extern void IEEE_802_15_4_PhyInit(void);


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_BindService
 *!< @brief                  :
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : -
 *!< Critical section YES/NO : NO
 */
extern void IEEE_802_15_4_BindService(IEEE_802_15_4_ServiceAccessPoint_t serviceId, IEEE_802_15_4_Service_t serviceType, void (*func)(void*));



/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GetTranceiverState
 *!< @brief                  : Returns the state of tranceiver read from chip.
 *!< Parameters              :
 *!<                   Input : -
 *!<                   Output: -
 *!< Return                  : State of the tranceiver
 *!< Critical section YES/NO : NO
 */
IEEE_802_15_4_PHY_Enum_t IEEE_802_15_4_GetTranceiverState(void);


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GetPPDU
 *!< @brief                  : Parse buffer according to PPDU description ( per IEEE Std 802.15.4-2003  Figure 16 )
 *!< Parameters              :
 *!<                   Input : source - buffer should be parsed in PPDU
 *!<                   Output:
 *!< Return                  : Parsed PPDU
 *!< Critical section YES/NO : NO
 */
extern IEEE_802_15_4_PPDU_t IEEE_802_15_4_GetPPDU(uint8_t* source);

#endif /* _IEEE_802_15_4_PHY_H_ */
