/*
 * Just blasts shit over the UART in polled mode.  Prints received stuff to
 * stdout.
 *
 * Author: Evan K. Friis, UW Madison
 *
 * Modified from Xilinx xuartlite_polled_example.c
 *
 */

/* $Id: xuartlite_polled_example.c,v 1.1.2.1 2009/11/24 05:14:24 svemula Exp $ */
/******************************************************************************
 *
 * (c) Copyright 2002-2009 Xilinx, Inc. All rights reserved.
 *
 * This file contains confidential and proprietary information of Xilinx, Inc.
 * and is protected under U.S. and international copyright and other
 * intellectual property laws.
 *
 * DISCLAIMER
 * This disclaimer is not a license and does not grant any rights to the
 * materials distributed herewith. Except as otherwise provided in a valid
 * license issued to you by Xilinx, and to the maximum extent permitted by
 * applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
 * FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
 * IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
 * MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
 * and (2) Xilinx shall not be liable (whether in contract or tort, including
 * negligence, or under any other theory of liability) for any loss or damage
 * of any kind or nature related to, arising under or in connection with these
 * materials, including for any direct, or any indirect, special, incidental,
 * or consequential loss or damage (including loss of data, profits, goodwill,
 * or any type of loss or damage suffered as a result of any action brought by
 * a third party) even if such damage or loss was reasonably foreseeable or
 * Xilinx had been advised of the possibility of the same.
 *
 * CRITICAL APPLICATIONS
 * Xilinx products are not designed or intended to be fail-safe, or for use in
 * any application requiring fail-safe performance, such as life-support or
 * safety devices or systems, Class III medical devices, nuclear facilities,
 * applications related to the deployment of airbags, or any other applications
 * that could lead to death, personal injury, or severe property or
 * environmental damage (individually and collectively, "Critical
 * Applications"). Customer assumes the sole risk and liability of any use of
 * Xilinx products in Critical Applications, subject only to applicable laws
 * and regulations governing limitations on product liability.
 *
 * THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
 * AT ALL TIMES.
 *
 ******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xuartlite_polled_example.c
 *
 * This file contains a design example using the UartLite driver (XUartLite) and
 * hardware device using the polled mode.
 *
 * @note
 *
 * The user must provide a physical loopback such that data which is
 * transmitted will be received.
 *
 * MODIFICATION HISTORY:
 * <pre>
 * Ver   Who  Date	 Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a jhl  02/13/02 First release
 * 1.00a sv   06/13/05 Minor changes to comply to Doxygen and coding guidelines
 * 2.00a ktn  10/20/09 Updated this example to wait for valid data in receive
 *		      fifo instead of Tx fifo empty to update receive buffer
 *		      and minor changes as per coding guidelines.
 * </pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xstatus.h"
#include "xuartlite.h"

#include "xintc.h"		
#include "xil_exception.h"
/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define UARTLITE_DEVICE_ID	XPAR_UARTLITE_0_DEVICE_ID

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the UartLite, this constant must be 16 bytes or less since
 * this is a single threaded non-interrupt driven example such that the
 * entire buffer will fit into the transmit and receive FIFOs of the UartLite.
 */
#define TEST_BUFFER_SIZE 16

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int UartLitePolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XUartLite UartLite;		/* Instance of the UartLite Device */

void print(char *str);
int putchar(int chr);
/*
 * The following buffers are used in this example to send and receive data
 * with the UartLite.
 */
u8 SendBuffer[TEST_BUFFER_SIZE];	/* Buffer for Transmitting Data */
u8 RecvBuffer[TEST_BUFFER_SIZE+1];	/* Buffer for Receiving Data */

/*****************************************************************************/
/**
 *
 * Main function to call the Uartlite polled example.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 ******************************************************************************/
int main(void)
{
  print("starting\n");
  int Status;

  /*
   * Run the UartLite polled example, specify the Device ID that is
   * generated in xparameters.h
   */
  Status = UartLitePolledExample(UARTLITE_DEVICE_ID);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  return XST_SUCCESS;

}


/****************************************************************************/
/**
 * This function does a minimal test on the UartLite device and driver as a
 * design example. The purpose of this function is to illustrate
 * how to use the XUartLite component.
 *
 * This function sends data and expects to receive the data thru the UartLite
 * such that a  physical loopback must be done with the transmit and receive
 * signals of the UartLite.
 *
 * This function polls the UartLite and does not require the use of interrupts.
 *
 * @param	DeviceId is the Device ID of the UartLite and is the
 *		XPAR_<uartlite_instance>_DEVICE_ID value from xparameters.h.
 *
 * @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
 *
 *
 * @note
 *
 * This function calls the UartLite driver functions in a blocking mode such that
 * if the transmit data does not loopback to the receive, this function may
 * not return.
 *
 ****************************************************************************/
int UartLitePolledExample(u16 DeviceId)
{
  int Status;
  unsigned int SentCount;
  int Index;

  /*
   * Initialize the UartLite driver so that it is ready to use.
   */
  Status = XUartLite_Initialize(&UartLite, DeviceId);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /*
   * Perform a self-test to ensure that the hardware was built correctly.
   */
  Status = XUartLite_SelfTest(&UartLite);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  char output_string[16] = "trapped in fctr\n";
  XUartLite_ResetFifos(&UartLite);

  /*
   * Initialize the send buffer bytes with a pattern to send and the
   * the receive buffer bytes to zero.
   */
  for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
    SendBuffer[Index] = output_string[Index];
    RecvBuffer[Index] = 0;
  }
  RecvBuffer[TEST_BUFFER_SIZE] = '\0';

  xil_printf("begin blast\n");

  /*
   * Send the buffer through the UartLite waiting til the data can be sent
   * (block), if the specified number of bytes was not sent successfully,
   * then an error occurred.
   */

  unsigned int idx = 0;

  while (1) {
    int sent = XUartLite_Send(&UartLite, SendBuffer, TEST_BUFFER_SIZE);
    SentCount += sent;
    if (sent != TEST_BUFFER_SIZE) {
      xil_printf("sent count wrong %x\n", sent);
      //return XST_FAILURE;
    }

    if (!(idx++)) {
      XUartLite_Stats stats;
      XUartLite_GetStats(&UartLite, &stats);
      xil_printf("CharRX %x\n", stats.CharactersReceived);
      xil_printf("CharTX %x\n", stats.CharactersTransmitted);
      xil_printf("RX Interrupts %x\n", stats.ReceiveInterrupts);
      xil_printf("RX Frame Err %x\n", stats.ReceiveFramingErrors);
      xil_printf("RX Overrun Err %x\n", stats.ReceiveOverrunErrors);
      xil_printf("RX Parity Err %x\n", stats.ReceiveParityErrors);
      xil_printf("TX Interrupts %x\n", stats.TransmitInterrupts);
    }

    /*
     * Receive the number of bytes which is transfered.
     * Data may be received in fifo with some delay hence we continuously
     * check the receive fifo for valid data and update the receive buffer
     * accordingly.
     */
    while (1) {
      // receive as long as there is shit to receive
      int recv = XUartLite_Recv(&UartLite,
          RecvBuffer,
          TEST_BUFFER_SIZE);
      if (recv) {
        xil_printf("received->");
        for (int i = 0; i < recv; ++i) {
          //xil_printf("0x%x,", RecvBuffer[i]);
        }
        xil_printf((const char*)RecvBuffer);
        xil_printf("\n");
      }
      if (!recv) {
        break;
      } 
    }
  }

  return XST_SUCCESS;
}


