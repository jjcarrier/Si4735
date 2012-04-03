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

//Define RDS block B decoding masks
#define SI4735_RDS_TYPE_MASK 0xF800
#define SI4735_RDS_TYPE_SHR 11
#define SI4735_RDS_TP 0x0400
#define SI4735_RDS_PTY_MASK 0x03E0
#define SI4735_RDS_PTY_SHR 5
#define SI4735_RDS_TA word(0x0010)
#define SI4735_RDS_MS word(0x0008)
#define SI4735_RDS_DI word(0x0004)
#define SI4735_RDS_DIPS_ADDRESS word(0x0003)
#define SI4735_RDS_TEXTAB word(0x0010)
#define SI4735_RDS_TEXT_ADDRESS word(0x000F)
#define SI4735_RDS_MJD_MASK word(0x0003)
#define SI4735_RDS_MJD_SHL 15
#define SI4735_RDS_PTYNAB word(0x0010)
#define SI4735_RDS_PTYN_ADDRESS word(0x0001)

//Define RDS CT (group 4A) decoding masks
#define SI4735_RDS_TIME_TZ_OFFSET 0x0000001FUL
#define SI4735_RDS_TIME_TZ_SIGN 0x00000020UL
#define SI4735_RDS_TIME_MINUTE_MASK 0x00000FC0UL
#define SI4735_RDS_TIME_MINUTE_SHR 6
#define SI4735_RDS_TIME_HOUR_MASK 0x0001F000UL
#define SI4735_RDS_TIME_HOUR_SHR 12
#define SI4735_RDS_TIME_MJD_MASK 0xFFFE0000UL
#define SI4735_RDS_TIME_MJD_SHR 17

//Define RDS group types
#define SI4735_GROUP_0A 0x00
#define SI4735_GROUP_0B 0x01
#define SI4735_GROUP_1A 0x02
#define SI4735_GROUP_1B 0x03
#define SI4735_GROUP_2A 0x04
#define SI4735_GROUP_2B 0x05
#define SI4735_GROUP_3A 0x06
#define SI4735_GROUP_3B 0x07
#define SI4735_GROUP_4A 0x08
#define SI4735_GROUP_4B 0x09
#define SI4735_GROUP_5A 0x0A
#define SI4735_GROUP_5B 0x0B
#define SI4735_GROUP_6A 0x0C
#define SI4735_GROUP_6B 0x0D
#define SI4735_GROUP_7A 0x0E
#define SI4735_GROUP_7B 0x0F
#define SI4735_GROUP_8A 0x10
#define SI4735_GROUP_8B 0x11
#define SI4735_GROUP_9A 0x12
#define SI4735_GROUP_9B 0x13
#define SI4735_GROUP_10A 0x14
#define SI4735_GROUP_10B 0x15
#define SI4735_GROUP_11A 0x16
#define SI4735_GROUP_11B 0x17
#define SI4735_GROUP_12A 0x18
#define SI4735_GROUP_12B 0x19
#define SI4735_GROUP_13A 0x1A
#define SI4735_GROUP_13B 0x1B
#define SI4735_GROUP_14A 0x1C
#define SI4735_GROUP_14B 0x1D
#define SI4735_GROUP_15A 0x1E
#define SI4735_GROUP_15B 0x1F

#endif
