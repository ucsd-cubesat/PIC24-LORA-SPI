/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and
    application code development.

    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.125
        Device            :  PIC24FJ128GA204
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.36B
        MPLAB 	          :  MPLAB X v5.20
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/pin_manager.h"
#include "mcc_generated_files/tmr1.h"
#include "lora.h"
#include <stdio.h>

/* Main application */
int main(void)
{
    printf( "\nEntered main program...\n\r" );
    char payload[25] = "Hello, world!";
    
    // initialize the development board
    SYSTEM_Initialize();

    // Boot up the LoRa module with a 25-character payload size
    lora_init( 25 );
    
    write_reg( REG_OP_MODE, LORA_SLEEP );     //put lora into sleep mode
    write_reg( REG_FIFO_TX_BASE_ADDR, 0x00 ); //use the whole stack for TX
    write_reg( REG_OP_MODE, LORA_STANDBY );   //LoRa now ready
    
    while(1) {
        
        //set SPI access to FIFO
        write_reg( REG_FIFO_ADDR_PTR, read_reg( REG_FIFO_TX_BASE_ADDR ) );
        
        //load payload into FIFO
        int i;
        for( i = 0; i < sizeof( payload ); i++ ) {
            write_reg( REG_FIFO, payload[i] );
        }
        
        //commence Tx
        write_reg( REG_OP_MODE, LORA_TX );
        
        //wait for Tx to end
        while( read_reg( REG_IRQ_FLAGS ) != FLAG_TX_DONE ) {
            
            //a 1 second wait while the timer has not finished
            while( !TMR1_GetElapsedThenClear() ) {
                
              //refresh the timer status
              TMR1_Tasks_16BitOperation();
            }
        };
        
        //confirm Tx, as in the chip should fall back into standby mode
        if( read_reg( REG_OP_MODE ) == LORA_STANDBY ) {
            
            //clear the status flags
            write_reg( REG_IRQ_FLAGS, CLEAR_IRQ_FLAGS );
            printf( "Transmitted payload.\n\r" );
            
            //blink the status LED (left) for success
            TMR1_Start();
            LED_L_SetHigh();
            while( !TMR1_GetElapsedThenClear() ) {
              TMR1_Tasks_16BitOperation();
            }
            LED_L_SetLow();
            
        }
        else { //reinit the lora module if anything goes wrong
            lora_init( 25 );
            printf( "LoRa was reinitialized.\n\r" );
        }
    }

    return 1;
}
