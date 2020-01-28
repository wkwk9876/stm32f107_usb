#ifndef __USBH_CE20_H
#define __USBH_CE20_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "usbh_core.h"


/* CH340 Class Codes */
#define USB_CE20_CLASS                                   		(0xff)

/*Class-Specific Request Codes*/
#define CDC_SEND_ENCAPSULATED_COMMAND                           0x00
#define CDC_GET_ENCAPSULATED_RESPONSE                           0x01
#define CDC_SET_COMM_FEATURE                                    0x02
#define CDC_GET_COMM_FEATURE                                    0x03
#define CDC_CLEAR_COMM_FEATURE                                  0x04

#define CDC_SET_AUX_LINE_STATE                                  0x10
#define CDC_SET_HOOK_STATE                                      0x11
#define CDC_PULSE_SETUP                                         0x12
#define CDC_SEND_PULSE                                          0x13
#define CDC_SET_PULSE_TIME                                      0x14
#define CDC_RING_AUX_JACK                                       0x15

#define CDC_SET_LINE_CODING                                     0x20
#define CDC_GET_LINE_CODING                                     0x21
#define CDC_SET_CONTROL_LINE_STATE                              0x22
#define CDC_SEND_BREAK                                          0x23

#define CDC_SET_RINGER_PARMS                                    0x30
#define CDC_GET_RINGER_PARMS                                    0x31
#define CDC_SET_OPERATION_PARMS                                 0x32
#define CDC_GET_OPERATION_PARMS                                 0x33  
#define CDC_SET_LINE_PARMS                                      0x34
#define CDC_GET_LINE_PARMS                                      0x35
#define CDC_DIAL_DIGITS                                         0x36
#define CDC_SET_UNIT_PARAMETER                                  0x37  
#define CDC_GET_UNIT_PARAMETER                                  0x38
#define CDC_CLEAR_UNIT_PARAMETER                                0x39
#define CDC_GET_PROFILE                                         0x3A

#define CDC_SET_ETHERNET_MULTICAST_FILTERS                      0x40
#define CDC_SET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x41
#define CDC_GET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x42
#define CDC_SET_ETHERNET_PACKET_FILTER                          0x43
#define CDC_GET_ETHERNET_STATISTIC                              0x44

#define CDC_SET_ATM_DATA_FORMAT                                 0x50  
#define CDC_GET_ATM_DEVICE_STATISTICS                           0x51
#define CDC_SET_ATM_DEFAULT_VC                                  0x52
#define CDC_GET_ATM_VC_STATISTICS                               0x53


/* wValue for SetControlLineState*/
#define CDC_ACTIVATE_CARRIER_SIGNAL_RTS                         0x0002
#define CDC_DEACTIVATE_CARRIER_SIGNAL_RTS                       0x0000
#define CDC_ACTIVATE_SIGNAL_DTR                                 0x0001
#define CDC_DEACTIVATE_SIGNAL_DTR                               0x0000

#define LINE_CODING_STRUCTURE_SIZE                              0x07



extern USBH_ClassTypeDef  CE20_Class;
#define USBH_CE20_CLASS    &CE20_Class

typedef enum
{
  CE20_IDLE_STATE= 0,
  CE20_SET_LINE_CODING_STATE,  
  CE20_GET_LAST_LINE_CODING_STATE,    
  CE20_TRANSFER_DATA, 
  CE20_ERROR_STATE,  
}
CE20_StateTypeDef;

/* States for CE20 State Machine */
typedef enum
{
  CE20_IDLE= 0,
  CE20_SEND_DATA,
  CE20_SEND_DATA_WAIT,
  CE20_RECEIVE_DATA,
  CE20_RECEIVE_DATA_WAIT,  
}
CE20_DataStateTypeDef;


/*Line coding structure*/
typedef union _CE20_LineCodingStructure
{
  uint8_t Array[LINE_CODING_STRUCTURE_SIZE];
  
  struct
  {
    
    uint32_t             dwDTERate;     /*Data terminal rate, in bits per second*/
    uint8_t              bCharFormat;   /*Stop bits
    0 - 1 Stop bit
    1 - 1.5 Stop bits
    2 - 2 Stop bits*/
    uint8_t              bParityType;   /* Parity
    0 - None
    1 - Odd
    2 - Even
    3 - Mark
    4 - Space*/
    uint8_t                bDataBits;     /* Data bits (5, 6, 7, 8 or 16). */
  }b;
}
CE20_LineCodingTypeDef;

/* Header Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description                    
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Identifier (ID) of functional 
      |                     |       |            | descriptor.
3     |  bcdCDC             | 2     |            |
      |                     |       |   Number   | USB Class Definitions for 
      |                     |       |            | Communication Devices Specification 
      |                     |       |            | release number in binary-coded 
      |                     |       |            | decimal
------|---------------------|-------|------------|------------------------------
*/
typedef struct _FunctionalDescriptorHeader
{
  uint8_t     bLength;            /*Size of this descriptor.*/
  uint8_t     bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t     bDescriptorSubType; /* Header functional descriptor subtype as*/
  uint16_t    bcdCDC;             /* USB Class Definitions for Communication
                                    Devices Specification release number in
                                  binary-coded decimal. */
}
CE20_HeaderFuncDesc_TypeDef;

/* Call Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description                    
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Call Management functional 
      |                     |       |            |  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports:
      |                     |       |            | D7..D2: RESERVED (Reset to zero)
      |                     |       |            | D1: 0 - Device sends/receives call
      |                     |       |            | management information only over
      |                     |       |            | the Communication Class
      |                     |       |            | interface.
      |                     |       |            | 1 - Device can send/receive call
      |                     \       |            | management information over a
      |                     |       |            | Data Class interface.
      |                     |       |            | D0: 0 - Device does not handle call
      |                     |       |            | management itself.
      |                     |       |            | 1 - Device handles call
      |                     |       |            | management itself.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which call management scenario is used. If bit
      |                     |       |            | D0 is reset to 0, then the value of bit D1 is
      |                     |       |            | ignored. In this case, bit D1 is reset to zero for
      |                     |       |            | future compatibility.
4     | bDataInterface      | 1     | Number     | Interface number of Data Class interface
      |                     |       |            | optionally used for call management.        
------|---------------------|-------|------------|------------------------------
*/
typedef struct _CallMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Call Management functional descriptor subtype*/
  uint8_t    bmCapabilities;      /* bmCapabilities: D0+D1 */
  uint8_t    bDataInterface;      /*bDataInterface: 1*/
}
CE20_CallMgmtFuncDesc_TypeDef;

/* Abstract Control Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description                    
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of functional descriptor, in bytes.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Abstract Control Management 
      |                     |       |            |  functional  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports ((A bit value of zero means that the
      |                     |       |            | request is not supported.) )
                                                   D7..D4: RESERVED (Reset to zero)
      |                     |       |            | D3: 1 - Device supports the notification
      |                     |       |            | Network_Connection.
      |                     |       |            | D2: 1 - Device supports the request
      |                     |       |            | Send_Break
      |                     |       |            | D1: 1 - Device supports the request
      |                     \       |            | combination of Set_Line_Coding,
      |                     |       |            | Set_Control_Line_State, Get_Line_Coding, and the
                                                   notification Serial_State.
      |                     |       |            | D0: 1 - Device supports the request
      |                     |       |            | combination of Set_Comm_Feature,
      |                     |       |            | Clear_Comm_Feature, and Get_Comm_Feature.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which requests/notifications are supported by
      |                     |       |            | a Communication Class interface with the
      |                     |       |            |   SubClass code of Abstract Control Model.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _AbstractCntrlMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Abstract Control Management functional
                                  descriptor subtype*/
  uint8_t    bmCapabilities;      /* The capabilities that this configuration supports */
}
CE20_AbstCntrlMgmtFuncDesc_TypeDef;

/* Union Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description                    
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Union functional 
      |                     |       |            |  descriptor subtype.
3     |  bMasterInterface   | 1     |   Constant | The interface number of the 
      |                     |       |            | Communication or Data Class interface
4     | bSlaveInterface0    | 1     | Number     | nterface number of first slave or associated
      |                     |       |            | interface in the union.        
------|---------------------|-------|------------|------------------------------
*/
typedef struct _UnionFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Union functional descriptor SubType*/
  uint8_t    bMasterInterface;   /* The interface number of the Communication or
                                 Data Class interface,*/
  uint8_t    bSlaveInterface0;   /*Interface number of first slave*/
}
CE20_UnionFuncDesc_TypeDef;

typedef struct _USBH_CE20InterfaceDesc
{
  CE20_HeaderFuncDesc_TypeDef           CDC_HeaderFuncDesc;
  CE20_CallMgmtFuncDesc_TypeDef         CDC_CallMgmtFuncDesc;
  CE20_AbstCntrlMgmtFuncDesc_TypeDef    CDC_AbstCntrlMgmtFuncDesc;
  CE20_UnionFuncDesc_TypeDef            CDC_UnionFuncDesc;  
}
CE20_InterfaceDesc_Typedef;

/* Structure for CE20 process */
typedef struct
{
  uint8_t              NotifPipe; 
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
}
CE20_CommItfTypedef ;

typedef struct
{
  uint8_t              InPipe; 
  uint8_t              OutPipe;
  uint8_t              OutEp;
  uint8_t              InEp;
  uint8_t              buff[8];
  uint16_t             OutEpSize;
  uint16_t             InEpSize;  
}
CE20_DataItfTypedef ;

/* Structure for CE20 process */
typedef struct _CE20_Process
{
  CE20_CommItfTypedef				CommItf;
  CE20_DataItfTypedef				DataItf;
  uint8_t							*pTxData;
  uint8_t							*pRxData; 
  uint32_t							 TxDataLength;
  uint32_t							 RxDataLength;	
  CE20_InterfaceDesc_Typedef 		CDC_Desc;
  CE20_LineCodingTypeDef 			LineCoding;
  CE20_LineCodingTypeDef 			*pUserLineCoding;  
  CE20_StateTypeDef					state;
  CE20_DataStateTypeDef				data_tx_state;
  CE20_DataStateTypeDef				data_rx_state; 
  uint8_t							Rx_Poll;
  //CE20_AttachStateTypeDef			attach_state;
  unsigned char 					buf[8];
}
CE20_HandleTypeDef;

USBH_StatusTypeDef	USBH_CE20_Transmit(USBH_HandleTypeDef *phost, 
									  uint8_t *pbuff, 
									  uint32_t length);

USBH_StatusTypeDef	USBH_CE20_Receive(USBH_HandleTypeDef *phost, 
									 uint8_t *pbuff, 
									 uint32_t length);

uint16_t			USBH_CE20_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);


void USBH_CE20_TransmitCallback(USBH_HandleTypeDef *phost);

void USBH_CE20_ReceiveCallback(USBH_HandleTypeDef *phost);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

