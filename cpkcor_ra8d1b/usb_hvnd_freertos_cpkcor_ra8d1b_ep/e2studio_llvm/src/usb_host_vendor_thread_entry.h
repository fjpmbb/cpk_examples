/***********************************************************************************************************************
 * File Name    : usb_host_vendor_thread_entry.h
 * Description  : Contains macros definitions.
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
#ifndef USB_HOST_VENDOR_THREAD_ENTRY_H_
#define USB_HOST_VENDOR_THREAD_ENTRY_H_

#define BUF_SIZE               (20)                // Buffer size
#define REQ_SIZE               (20)                // Request buffer size
#define ADDRESS                (1)                 // USB address
#define START_PIPE             (1)                 // Start pipe number
#define END_PIPE               (USB_PIPE9 + 1)     // Total pipe

/* for Vendor Class Request */
#define USB_SET_VENDOR_NO_DATA (0x0000U)
#define USB_SET_VENDOR         (0x0100U)
#define USB_GET_VENDOR         (0x0200U)
#define SET_VENDOR_NO_DATA     (USB_SET_VENDOR_NO_DATA | USB_HOST_TO_DEV | USB_VENDOR | USB_INTERFACE)
#define SET_VENDOR             (USB_SET_VENDOR | USB_HOST_TO_DEV | USB_VENDOR | USB_INTERFACE)
#define GET_VENDOR             (USB_GET_VENDOR | USB_DEV_TO_HOST | USB_VENDOR | USB_INTERFACE)
#define DELAY                  (10U)               // Delay for print
#define USB_VALUE_FF           (0xFF)              // FF macro
#if defined (BOARD_RA6M3_EK) || defined (BOARD_RA6M3G_EK) || defined (BOARD_RA8D1_EK)
#define USB_APL_MXPS           (512U)              // Max packet size high speed
#else
#define USB_APL_MXPS           (64U)               // Max packet size full speed
#endif
#define EP_INFO "\r\nThe project demonstrates the basic functionalities of USB Host Vendor class driver\r\n"\
                "on Renesas RA MCUs based on Renesas FSP. In this example, the application will configure\r\n"\
                "the MCU as a Vendor Host device. This Host device will be connected to the USB Peripheral\r\n"\
                "Vendor device which is another RA board.\n\n"\
                "After the enumeration is completed, the Vendor Host board will write15 Bytes of data to the\r\n"\
                "Vendor Peripheral board and read back the same data from the Vendor Peripheral board.The Vendor\r\n"\
                "Host and Vendor Peripheral applications are designed to continuously transfer data between both\r\n"\
                "the boards. User will be able to see all the operation sequence and status on JLink RTTViewer.\r\n"

#endif /* USB_HOST_VENDOR_THREAD_ENTRY_H_ */
