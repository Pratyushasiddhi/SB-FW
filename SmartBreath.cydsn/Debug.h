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
#ifndef _SB_DEBUG_H_
#define _SB_DEBUG_H_
    
#include <stdio.h>
#define STR		0
#define HEX		1
#define DEC		2

#define YES 	1
#define NO  	0

#include <stdio.h>

#define _DEBUG_ERROR 1
#define _DEBUG_INFO  2
#define PSOC_DEBUG_LEVEL _DEBUG_ERROR

#ifdef NDEBUG
/* NO debug */
#define PSOC_LOG_ERR(fmt, ...)
#define PSOC_LOG_INFO(fmt, ...)
#define PSOC_LOG_DEGUG_INFO(fmt, ...)

#else       
#if (PSOC_DEBUG_LEVEL == _DEBUG_ERROR)
#define PSOC_LOG_ERR(fmt, ...) \
     do { \
        char buf[256];  \
        sprintf(buf, "%s: %d: " fmt, \
             __FILE__,__LINE__,##__VA_ARGS__); \
     } while(0);

#define PSOC_LOG_INFO(fmt, ...)
#define PSOC_LOG_DEGUG_INFO(fmt, ...)
    
#elif (PSOC_DEBUG_LEVEL == _DEBUG_INFO)
#define PSOC_LOG_ERR(fmt, ...) \
     do { \
        char buf[256];  \
        sprintf(buf, "%s: %d: " fmt, \
             __FILE__,__LINE__,##__VA_ARGS__); \
        UART_1_PutString(buf); \
    } while(0);

#define PSOC_LOG_INFO(fmt, ...) \
     do { \
        char buf[256];  \
        sprintf(buf, "%s: %d: " fmt, \
             __FILE__,__LINE__,##__VA_ARGS__); \
        UART_1_PutString(buf); \
    } while(0);
    
#define PSOC_LOG_DEGUG_INFO(fmt, ...) \
     do { \
        char buf[256];  \
        sprintf(buf, fmt, \
             ##__VA_ARGS__); \
        UART_1_PutString(buf); \
    } while(0);    

#endif

    
#endif     
#endif
/* [] END OF FILE */
