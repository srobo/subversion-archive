/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        usbcfg.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ********************************************************************/

#ifndef USBCFG_H
#define USBCFG_H

/** D E F I N I T I O N S *******************************************/
#define EP0_BUFF_SIZE           16   // 8, 16, 32, or 64
#define MAX_NUM_INT             1   // For tracking Alternate Setting

/* Parameter definitions are defined in usbdrv.h */
#define MODE_PP                 _PPBM0				// ping pong buffer mode 0
#define UCFG_VAL                _PUEN|_TRINT|_FS|MODE_PP	// internal pull-up resistor, internal transreceiver, FS USB mode, ppong b0

#define USE_SELF_POWER_SENSE_IO
#define USE_USB_BUS_SENSE_IO

/** D E V I C E  C L A S S  U S A G E *******************************/
#define USB_USE_MSD

/*
 * MUID = Microchip USB Class ID
 * Used to identify which of the USB classes owns the current
 * session of control transfer over EP0
 */
#define MUID_NULL               0
#define MUID_USB9               1
#define MUID_HID                2
#define MUID_CDC                3
#define MUID_MSD				4

/** E N D P O I N T S  A L L O C A T I O N **************************/
/*
 * See usbmmap.c for an explanation of how the endpoint allocation works
 */
/* MSD*/

#define MSD_INTF_ID             0x00
#define MSD_UEP                 UEP1
#define MSD_BD_OUT              ep1Bo
#define MSD_OUT_EP_SIZE     	64			/*Max endpoint size for F/S since 4550 cannot do H/S*/
#define MSD_BD_IN               ep1Bi
#define MSD_IN_EP_SIZE      	64
#define MSD_NUM_OF_DSC          1

/* MSD macros */
//because we donot have a descriptor for MSD class separatedly and msd_i00a00 is not defined.
//#define mUSBGetMSDDscAdr(ptr)               \
//{                                           \
//    if(usb_active_cfg == 1)                 \
//        ptr = (rom byte*)&cfg01.msd_i00a00; \
//}


//#define mUSBGetHIDRptDscAdr(ptr)            \
//{                                           \
//    if(usb_active_cfg == 1)                 \
//        ptr = (rom byte*)&hid_rpt01;        \
//}
//
//#define mUSBGetHIDRptDscSize(count)         \
//{                                           \
//    if(usb_active_cfg == 1)                 \
//        count = sizeof(hid_rpt01);          \
//}


#define MAX_EP_NUMBER 1           // UEP1

#endif //USBCFG_H
