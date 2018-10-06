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
#ifndef _COMMON_H_
#define _COMMON_H_
    
#define TRUE                1
#define FALSE               0
#define ZERO	    		0
    
#define ON                  (1u)
#define OFF                 (0u)
    
typedef unsigned long		UINT32;
typedef unsigned short		UINT16;
typedef unsigned char		UINT8;
typedef signed   long		INT32;
typedef signed   short		INT16;
typedef signed   char		INT8;
typedef unsigned long long	UINT64;
typedef signed   long long	INT64;

#define RED_INDEX						0
#define GREEN_INDEX						1
#define BLUE_INDEX						2
#define INTENSITY_INDEX					3
    
typedef enum
{
	eO2PLUS			        = 0,	//!< Idle Mode
	eO2MINUS        	    = 1,	//!< Forward Rotation Mode
	eRESERVED_1	            = 2,	//!< Reverse Rotation Mode
    eRESERVED_2             = 3,
}E_Q2;
#endif //_COMMON_H_    
/* [] END OF FILE */
