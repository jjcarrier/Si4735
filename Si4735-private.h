/* Arduino Si4735 Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/Si4735/blob/master/README
 *
 * This library is for use with the SparkFun Si4735 Shield or Breakout Board.
 * See the example sketches to learn how to use the library in your code.
 *
 * This file contains definitions that are only used by the Si4735 class and
 * shouldn't be needed by most users.
 */

#ifndef _SI4735_PRIVATE_H_INCLUDED
#define _SI4735_PRIVATE_H_INCLUDED

//Define Si4735 SPI Command preambles
#define SI4735_CP_WRITE8 0x48
#define SI4735_CP_READ1_SDIO 0x80
#define SI4735_CP_READ16_SDIO 0xC0
#define SI4735_CP_READ1_GPO1 0xA0
#define SI4735_CP_READ16_GPO1 0xE0

//Define Si4735 I2C Addresses
#define SI4735_I2C_ADDR_L (0x22 >> 1)
#define SI4735_I2C_ADDR_H (0xC6 >> 1)

#endif
