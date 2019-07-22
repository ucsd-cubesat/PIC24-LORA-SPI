/* 
 * File:   lora.c
 * Author: Christopher Madrigal
 *
 * Created on July 17, 2019
 */

#include <xc.h>
#include "lora.h"
#include "mcc_generated_files/spi1.h"
#include "mcc_generated_files/pin_manager.h"
#include <stdio.h>

/*
 * Boots the LoRa module.
 * Requires SPI to be initialized.
 */
void lora_init( char bufferSize ) {
  
  //reset lora device w/ posedge
  RST_SetHigh();

  //get current mode from the OpMode register
  uint8_t bootmode = read_reg( REG_OP_MODE );

  //Check the boot mode is LoRa
  //If not, put into FSK sleep mode
  if( !( bootmode & LORA_MODE ) ) {
    write_reg( REG_OP_MODE, FSK_SLEEP );
  }
  
  //LoRa mode is activated only while in sleep mode
  //While asleep, put into LoRa sleep mode, then standby
  write_reg( REG_OP_MODE, LORA_SLEEP );
  write_reg( REG_OP_MODE, LORA_STANDBY );

  //verify LoRa is in boot mode
  bootmode = read_reg( REG_OP_MODE );
  if( bootmode != LORA_STANDBY ) {
      printf( "Could not initialize LORA: 0x%x.\n\r", bootmode );
      lora_close();
  }
  else {
      printf( "LORA on standby.\n\r" );
      LED_R_SetHigh();
      
      //if all is good, set the payload size
      write_reg( REG_PAYLOAD_LEN, bufferSize );
      write_reg( REG_MAX_PAYLOAD_LEN, bufferSize );
  }
}

/*
 * Puts the LoRa module into sleep mode.
 * Requires chip to have been initialized
 */
void lora_close() {
	write_reg( REG_OP_MODE, LORA_SLEEP );
    LED_R_SetLow();
}

/*
 * Reads from the specified register.
 * Returns the value in the register.
 * 
 * Note: Transaction occurs as two byte exchanges
 * 
 *   CS: ?|________________________________________________________________________________________________|?
 *  CLK: ___|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|___
 * MOSI: X  0  X  A6 X  A5 X  A4 X  A3 X  A2 X  A1 X  A0 X   0 X   0 X   0 X   0 X   0 X   0 X   0 X   0 XXXX
 * MISO: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  D7 X  D6 X  D5 X  D4 X  D3 X  D2 X  D1 X  D0 XXXX
 * 
 * - First bit is 0 for read, followed by 7 bits for the register address
 *     Chip sends garbage during this phase, which we ignore
 * - Next byte sent is 0 to be safe
 *     Chip sends the byte that was in the register
 */
uint8_t read_reg( uint8_t reg ) {
    
    //Chip is selected when line is low
    CS_SetLow();
    
    //send register address in read mode, ignore response
    SPI1_Exchange8bit( reg );
    
    //return response, send only 0's
    uint8_t ret = SPI1_Exchange8bit( 0 );
    
    //deselect chip with line high
    CS_SetHigh();
    
    return ret;
}

/*
 * Writes to the specified register.
 * Returns the previous value in the register.
 * 
 * Note: Transaction occurs as two byte exchanges
 * 
 *   CS: ?|________________________________________________________________________________________________|?
 *  CLK: ___|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|__|??|___
 * MOSI: X  1  X  A6 X  A5 X  A4 X  A3 X  A2 X  A1 X  A0 X  D7 X  D6 X  D5 X  D4 X  D3 X  D2 X  D1 X  D0 XXXX
 * MISO: XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX  D7 X  D6 X  D5 X  D4 X  D3 X  D2 X  D1 X  D0 XXXX
 * 
 * - First bit is 1 for write, followed by 7 bits for the register address
 *     Chip sends garbage during this phase, which we ignore
 * - Next byte sent is the data we want to write
 *     Chip sends the byte that was in the register
 */
uint8_t write_reg( uint8_t reg, uint8_t data ) {
    
    CS_SetLow();
    
    //send register address in write mode, ignore response
    // or'ing the address with 0x80 puts a 1 at the front
    SPI1_Exchange8bit( reg | 0x80 );
    
    //send data, return response
    uint8_t ret = SPI1_Exchange8bit( data );
    
    CS_SetHigh();
    
    return ret;
}
