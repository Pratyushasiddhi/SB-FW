/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "Common.h"
#include "Bluetooth.h"

/**************************Variable Declarations*****************************/
/* 'connectionHandle' stores connection parameters */
CYBLE_CONN_HANDLE_T  connectionHandle;

/* 'ValveHandle' stores Valve control data parameters */
CYBLE_GATT_HANDLE_VALUE_PAIR_T		ValveHandle;	

/* Array to store the present Q2 Valve control data. The 4 bytes 
* of the array represent {Q2+,Q2-,Reserved ,Reserved} */
uint8 Q2ValveData[Q2_VALVE_CHAR_DATA_LEN];

/* This flag is used to let application send a L2CAP connection update request
* to Central device */
static uint8 isConnectionUpdateRequested = TRUE;

/* 'restartAdvertisement' flag provides the present state of power mode in firmware */
uint8 restartAdvertisement = FALSE;
/* This flag is used by application to know whether a Central 
* device has been connected. This is updated in BLE event callback 
* function*/
E_BLE_STATUS eBleStatus ;

/*This flag is set when the Central device writes to CCCD of the 
* Q2Valve Characteristic to enable notifications */
uint8 Q2ValveNotifications = FALSE;

/* Connection Parameter update values. These values are used by the BLE component
* to update the connection parameters, including connection interval, to desired 
* value */
static CYBLE_GAP_CONN_UPDATE_PARAM_T ConnectionParam =
{
    CONN_PARAM_UPDATE_MIN_CONN_INTERVAL,  		      
    CONN_PARAM_UPDATE_MAX_CONN_INTERVAL,		       
    CONN_PARAM_UPDATE_SLAVE_LATENCY,			    
    CONN_PARAM_UPDATE_SUPRV_TIMEOUT 			         	
};

uint8 Q2ValveCCCDvalue[2];


/* Handle value to update the CCCD */
CYBLE_GATT_HANDLE_VALUE_PAIR_T Q2ValveNotificationCCCDhandle;

/* Status flag for the Stack Busy state. This flag is used to notify the application 
* whether there is stack buffer free to push more data or not */
uint8 busyStatus = 0;



/*******************************************************************************
* Function Name: UpdateConnectionParam
********************************************************************************
* Summary:
*        Send the Connection Update Request to Client device after connection 
* and modify the connection interval for low power operation.
*
* Parameters:
*	void
*
* Return:
*  void
*
*******************************************************************************/
void UpdateConnectionParam(void)
{
	/* If device is connected and Update connection parameter not updated yet,
	* then send the Connection Parameter Update request to Client. */
    if( (eBLE_CONNECTED == eBleStatus)  && isConnectionUpdateRequested)
	{
		/* Reset the flag to indicate that connection Update request has been sent */
		isConnectionUpdateRequested = FALSE;
		
		/* Send Connection Update request with set Parameter */
		CyBle_L2capLeConnectionParamUpdateRequest(connectionHandle.bdHandle, &ConnectionParam);
	}
}

/*******************************************************************************
* Function Name: SendDataOverRGBledNotification
********************************************************************************
* Summary:
*        Send RGB LED data as BLE Notifications. This function updates
* the notification handle with data and triggers the BLE component to send 
* notification
*
* Parameters:
*  uint8 *rgbLedData:	pointer to an array containing RGB color and Intensity values
*  uint8 len: length of the array
*
* Return:
*  void
*
*******************************************************************************/
void SendDataOverQ2ValveNotification(uint8 *Q2ValveData, uint8 len)
{
	/* 'rgbLednotificationHandle' stores RGB LED notification data parameters */
	CYBLE_GATTS_HANDLE_VALUE_NTF_T 	Q2ValvenotificationHandle;
	
	/* If stack is not busy, then send the notification */
	if(busyStatus == CYBLE_STACK_STATE_FREE)
	{
		/* Update notification handle with CapSense slider data*/
		Q2ValvenotificationHandle.attrHandle = CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CHAR_HANDLE;				
		Q2ValvenotificationHandle.value.val = Q2ValveData;
		Q2ValvenotificationHandle.value.len = len;
		
		/* Send the updated handle as part of attribute for notifications */
		CyBle_GattsNotification(connectionHandle,&Q2ValvenotificationHandle);
	}
}




/*******************************************************************************
* Function Name: CustomEventHandler
********************************************************************************
* Summary:
*        Call back event function to handle various events from BLE stack
*
* Parameters:
*  uint32 event:		event returned
*  void * eventParam:	link to value of the events returned
*
* Return:
*  void
*
*******************************************************************************/
void CustomEventHandler(uint32 event, void * eventParam)
{
	/* Local variable to store the data received as part of the Write request 
	* events */
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;

    switch(event)
    {
        case CYBLE_EVT_STACK_ON:
			/* This event is received when component is Started */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
			
		case CYBLE_EVT_TIMEOUT:
			/* Event Handling for Timeout  */
	
			break;
        
		/**********************************************************
        *                       GAP Events
        ***********************************************************/
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			
			/* If the current BLE state is Disconnected, then the Advertisement
			* Start Stop event implies that advertisement has stopped */
			if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
			{
				/* Set restartAdvertisement flag to allow calling Advertisement 
				* API from main function */
				restartAdvertisement = TRUE;
			}
			break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED: 					
			/* This event is received when device is connected over GAP layer */
			break;

			
        case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* This event is received when device is disconnected */
			
			/* Set restartAdvertisement flag to allow calling Advertisement 
			* API from main function */
			restartAdvertisement = TRUE;
			
			break;
        
		/**********************************************************
        *                       GATT Events
        ***********************************************************/
        case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received when device is connected over GATT level */
			
			/* Update attribute handle on GATT Connection*/
            connectionHandle = *(CYBLE_CONN_HANDLE_T  *)eventParam;
			
			/* This flag is used in application to check connection status */
            eBleStatus = eBLE_CONNECTED;
			break;
        
        case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received when device is disconnected */
			
			/* Update deviceConnected flag*/
			eBleStatus = eBLE_DISCONNECTED;
			
			
			/* Reset RGB notification flag to prevent further notifications
			 * being sent to Central device after next connection. */
			Q2ValveNotifications = FALSE;
	
    		/* Write the present RGB notification status to the local variable */
    		Q2ValveCCCDvalue[0] = Q2ValveNotifications;
    		Q2ValveCCCDvalue[1] = 0x00;
    		
    		/* Update CCCD handle with notification status data*/
    		Q2ValveNotificationCCCDhandle.attrHandle = CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CUSTOM_DESCRIPTOR_DESC_HANDLE;
    		
            Q2ValveNotificationCCCDhandle.value.val = Q2ValveCCCDvalue;
    		Q2ValveNotificationCCCDhandle.value.len = CCCD_DATA_LEN;
    		
    		/* Report data to BLE component for sending data when read by Central device */
    		CyBle_GattsWriteAttributeValue(&Q2ValveNotificationCCCDhandle, ZERO, &connectionHandle, CYBLE_GATT_DB_PEER_INITIATED);
			
			/* Reset the color coordinates */
			Q2ValveData[RED_INDEX] = ZERO;
            Q2ValveData[GREEN_INDEX] = ZERO;
            Q2ValveData[BLUE_INDEX] = ZERO;
            Q2ValveData[INTENSITY_INDEX] = ZERO;
			//UpdateRGBled();
            //TODO:: Update to Motor
			/* Reset the isConnectionUpdateRequested flag to allow sending
			* connection parameter update request in next connection */
			isConnectionUpdateRequested = TRUE;
			
			/* Set the flag to allow system to go to Deep Sleep */
			//shut_down_led = TRUE;
			
			break;
            
        case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received when Central device sends a Write command on an Attribute */
            wrReqParam = (CYBLE_GATTS_WRITE_REQ_PARAM_T *) eventParam;
			if(CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CUSTOM_DESCRIPTOR_DESC_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/* Extract the Write value sent by the Client for RGB LED CCCD */
                if(wrReqParam->handleValPair.value.val[CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CUSTOM_DESCRIPTOR_DESC_INDEX] == TRUE)
                {
                    Q2ValveNotifications = TRUE;
                }
                else if(wrReqParam->handleValPair.value.val[CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CUSTOM_DESCRIPTOR_DESC_INDEX] == FALSE)
                {
                    Q2ValveNotifications = FALSE;
                }
				
        		/* Write the present RGB notification status to the local variable */
        		Q2ValveCCCDvalue[0] = Q2ValveNotifications;
        		Q2ValveCCCDvalue[1] = 0x00;
        		
        		/* Update CCCD handle with notification status data*/
        		Q2ValveNotificationCCCDhandle.attrHandle = CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CUSTOM_DESCRIPTOR_DESC_HANDLE;
        		Q2ValveNotificationCCCDhandle.value.val = Q2ValveCCCDvalue;
        		Q2ValveNotificationCCCDhandle.value.len = CCCD_DATA_LEN;
        		
        		/* Report data to BLE component for sending data when read by Central device */
        		CyBle_GattsWriteAttributeValue(&Q2ValveNotificationCCCDhandle, ZERO, &connectionHandle, CYBLE_GATT_DB_PEER_INITIATED);
				
				/* Update the RGB LED Notification attribute with new color coordinates */
				SendDataOverQ2ValveNotification(Q2ValveData, Q2_VALVE_CHAR_DATA_LEN);
            }
			
			/* Check if the returned handle is matching to RGB LED Control Write Attribute and extract the RGB data*/
            if(CYBLE_SMART_BREATH_SMART_BREATH_CONTROL_CHAR_HANDLE == wrReqParam->handleValPair.attrHandle)
            {
				/* Extract the Write value sent by the Client for RGB LED Color characteristic */
                Q2ValveData[eO2PLUS] = wrReqParam->handleValPair.value.val[eO2PLUS];
                Q2ValveData[eO2MINUS] = wrReqParam->handleValPair.value.val[eO2MINUS];
                Q2ValveData[eRESERVED_1] = wrReqParam->handleValPair.value.val[eRESERVED_1];
                Q2ValveData[eRESERVED_2] = wrReqParam->handleValPair.value.val[eRESERVED_2];
                
                
			    if( (Q2ValveData[eO2PLUS]) && (!Q2ValveData[eO2MINUS]) )
                {
                    O2_PLUS_Write(OFF);
                    GREEN_Write(OFF);
                    O2_MINUS_Write(ON);
                    RED_Write(ON);
                    
                }
                else if( (!Q2ValveData[eO2PLUS]) && (Q2ValveData[eO2MINUS]) )
                {
                    O2_PLUS_Write(ON);
                    GREEN_Write(ON);
                    O2_MINUS_Write(OFF);
                    RED_Write(OFF);
                    
                }
                else
                {
                    O2_PLUS_Write(ON);
                    O2_MINUS_Write(ON);
                    RED_Write(ON);
                    GREEN_Write(ON);
                }
                
                
            }
			
			/* Send the response to the write request received. */
			CyBle_GattsWriteRsp(connectionHandle);
			
			break;
			
		case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
				/* If L2CAP connection parameter update response received, reset application flag */
            	isConnectionUpdateRequested = FALSE;
            break;
			
		case CYBLE_EVT_STACK_BUSY_STATUS:
			/* This event is generated when the internal stack buffer is full and no more
			* data can be accepted or the stack has buffer available and can accept data.
			* This event is used by application to prevent pushing lot of data to stack. */
			
			/* Extract the present stack status */
            busyStatus = * (uint8*)eventParam;
            break;
			
        default:

       	 	break;
    }   	/* switch(event) */
}

/*******************************************************************************
* Function Name: InitializeSystem
********************************************************************************
* Summary:
*        Start the components and initialize system 
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void InitializeBluetoothSystem(void)
{
	/* Enable global interrupt mask */
	CyGlobalIntEnable; 
			
	/* Start BLE component and register the CustomEventHandler function. This 
	* function exposes the events from BLE component for application use */
    CyBle_Start(CustomEventHandler);
	
}




/* [] END OF FILE */
