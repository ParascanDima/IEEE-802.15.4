/*
 * ieee_802_15_4.c
 *
 *  Created on: 25 сент. 2018 г.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ieee_802_15_4.h"

/**************Private Macro Definitions*****************/

#define BYTEPTR(x)              ((uint8_t*)&(x))

#define COPY_BYTES_TO(variable, n, fromAddress)   for(index = 0; index < n; ++index){ \
        BYTEPTR(variable)[index] = BYTEPTR(fromAddress+index); \
}

/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/




/**************Public Variable Definitions***************/



/**************Private Function Definitions**************/





/**************Public Function Definitions***************/


/****************************************************************************************
 *!< Function                : IEEE_802_15_4_GetMPDU
 *!< @brief                  : Parse buffer according to MPDU description
 *!< Parameters              :
 *!<                   Input : source - buffer should be parsed in MPDU
 *!<                   Output:
 *!< Return                  : Parsed MPDU
 *!< Critical section YES/NO : NO
 */
IEEE_802_15_4_PPDU_t IEEE_802_15_4_GetMPDU(uint8_t* source, uint8_t frameLength)
{
    uint8_t  index;
    uint32_t dstPANIDAddr = 0;
    uint32_t dstAddrValueAddr = 0;
    uint32_t srcPANIDAddr = 0;
    uint32_t srcAddrValueAddr = 0;
#ifdef IEEE_802_15_4_2011_COMPLIANT
    uint16_t auxSecHeaderAddr = 0;
#endif
    uint32_t payloadAddr = 0;

    const uint32_t frameControlAddr = (uint32_t)&source[0];
    const uint32_t seqNumAddr  = frameControlAddr + 2;

    Rx.payloadLength = 0;

    /**COPY_BYTES_TO(Rx.frameLength, 1, &source[0]);  // Frame length byte shows the length of packet except LQI and RSSI bytes  */
    COPY_BYTES_TO(Rx.frameControl.value, 2, frameControlAddr);
    COPY_BYTES_TO(Rx.sequenceNumber, 1, seqNumAddr);

    if(Rx.frameControl.Field.dstAddrMode != IEEE_802_15_4_NO_ADDR_FIELD){

        dstPANIDAddr = seqNumAddr + 1;
        COPY_BYTES_TO(Rx.dstPANID, 2, dstPANIDAddr);

        dstAddrValueAddr = dstPANIDAddr + 2;

        switch (Rx.frameControl.Field.dstAddrMode)
        {
            case IEEE_802_15_4_SHORT_ADDR_FIELD:
                COPY_BYTES_TO(Rx.dstAddr, 2, dstAddrValueAddr);
                srcPANIDAddr = dstAddrValueAddr + 2;
                break;

            case IEEE_802_15_4_LONG_ADDR_FIELD:
                COPY_BYTES_TO(Rx.dstAddr, 8, dstAddrValueAddr);
                srcPANIDAddr = dstAddrValueAddr + 8;
                break;

            default:
                return;
        }
    }
    else{
        srcPANIDAddr = seqNumAddr + 1;
    }

    if(Rx.frameControl.Field.srcAddrMode != IEEE_802_15_4_NO_ADDR_FIELD){

        if(Rx.frameControl.Field.intraPAN == 0){
            COPY_BYTES_TO(Rx.srcPANID, 2, srcPANIDAddr);
            srcAddrValueAddr = srcPANIDAddr + 2;
        }
        else{
            Rx.srcPANID = Rx.dstPANID;
            srcAddrValueAddr = srcPANIDAddr;
        }


        switch (Rx.frameControl.Field.dstAddrMode)
        {
            case IEEE_802_15_4_SHORT_ADDR_FIELD:
                COPY_BYTES_TO(Rx.srcAddr, 2, srcAddrValueAddr);
#ifdef IEEE_802_15_4_2011_COMPLIANT
                auxSecHeaderAddr = srcAddrValueAddr + 2;
#else
                payloadAddr = srcAddrValueAddr + 2;
#endif
                break;

            case IEEE_802_15_4_LONG_ADDR_FIELD:
                COPY_BYTES_TO(Rx.srcAddr, 8, srcAddrValueAddr);
#ifdef IEEE_802_15_4_2011_COMPLIANT
                auxSecHeaderAddr = srcAddrValueAddr + 8;
#else
                payloadAddr = srcAddrValueAddr + 8;
#endif
                break;

            default:
                return;
        }
    }
    else{
#ifdef IEEE_802_15_4_2011_COMPLIANT
        auxSecHeaderAddr = srcPANIDAddr;
#else
        payloadAddr = srcPANIDAddr;
#endif
    }
#ifdef IEEE_802_15_4_2011_COMPLIANT
    if(Rx.frameControl.Field.securityEnabled){

        COPY_BYTES_TO(Rx.auxSecurityHeader.securityControl.value, 1, auxSecHeaderAddr);
        COPY_BYTES_TO(Rx.auxSecurityHeader.frameCounter, 4, auxSecHeaderAddr+1);

        auxSecHeaderAddr += (4 + 1);

        if(Rx.auxSecurityHeader.securityControl.Field.keyIdMode != IEEE_802_15_4_KEY_ID_MODE_IMPLICIT){

            switch (Rx.auxSecurityHeader.securityControl.Field.keyIdMode)
            {
                case IEEE_802_15_4_KEY_ID_MODE_1BYTE:
                    COPY_BYTES_TO(Rx.auxSecurityHeader.keyID.keyIndex, 1, auxSecHeaderAddr);
                    payloadAddr = auxSecHeaderAddr + 1;
                break;


                case IEEE_802_15_4_KEY_ID_MODE_4BYTE:

                    COPY_BYTES_TO(Rx.auxSecurityHeader.keyID.keySource, 4, auxSecHeaderAddr);
                    COPY_BYTES_TO(Rx.auxSecurityHeader.keyID.keyIndex, 1, auxSecHeaderAddr+4);
                    payloadAddr = auxSecHeaderAddr + (4+1);
                break;


                case IEEE_802_15_4_KEY_ID_MODE_8BYTE:

                    COPY_BYTES_TO(Rx.auxSecurityHeader.keyID.keySource, 8, auxSecHeaderAddr);
                    COPY_BYTES_TO(Rx.auxSecurityHeader.keyID.keyIndex, 1, auxSecHeaderAddr+EIGHT_BYTES);
                    payloadAddr = auxSecHeaderAddr + (8+1);
                break;
            }

        }
    }
    else{
        payloadAddr = auxSecHeaderAddr;
    }
#endif

    Rx.headerLength = payloadAddr - RX_FIFO_START_ADDR;


    for (index = 0; index < (Rx.frameLength-1)-Rx.headerLength; ++index)
    {
        Rx.payload[Rx.payloadLength++] = highRead(payloadAddr+index);
    }

    COPY_BYTES_TO(Rx.fcs, 2, payloadAddr+Rx.payloadLength);
    COPY_BYTES_TO(Rx.lqi, 1, payloadAddr+Rx.payloadLength+2);
    COPY_BYTES_TO(Rx.rssi, 1, payloadAddr+Rx.payloadLength+2+1);
}
