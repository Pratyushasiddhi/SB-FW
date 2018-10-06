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
#include "Debug.h"
#include "Bluetooth.h"


extern E_BLE_STATUS eBleStatus ;
extern uint8 restartAdvertisement;

void Initialize ()
{
    O2_MINUS_Write(ON);
    O2_PLUS_Write(ON);
    
    RED_Write(ON);
    GREEN_Write(ON);
}
int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    
    InitializeBluetoothSystem();
    Initialize();
    
    for(;;)
    {
       /*Process event callback to handle BLE events. The events generated and 
		* used for this application are inside the 'CustomEventHandler' routine*/
        CyBle_ProcessEvents();
        
        if(eBLE_CONNECTED == eBleStatus )
        {
            /* After the connection, send new connection parameter to the Client device 
			* to run the BLE communication on desired interval. This affects the data rate 
			* and power consumption. High connection interval will have lower data rate but 
			* lower power consumption. Low connection interval will have higher data rate at
			* expense of higher power. This function is called only once per connection. */
			UpdateConnectionParam();
            
        }
        if(restartAdvertisement)
		{
			/* Reset 'restartAdvertisement' flag*/
			restartAdvertisement = FALSE;
           /* Start Advertisement and enter Discoverable mode*/
			CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);

		}
    }
}

/* [] END OF FILE */
