/* Arduino Si4735 Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/Si4735/blob/master/README
 *
 * This library is for use with the SparkFun Si4735 Shield or Breakout Board.
 * See the example sketches to learn how to use the library in your code.
 *
 * This is the main include file for the library.
 *
 * #define SI4735_DEBUG to get serial console dumps of commands sent and
 * responses received from the chip.
 * #define SI4735_NOI2C or SI4735_NOSPI to exclude I2C or SPI code; please
 * note that selecting an operation mode that has been excluded will result
 * in undefined behaviour.
 */

#ifndef _SI4735_H_INCLUDED
#define _SI4735_H_INCLUDED

/* linking with Wire causes widespread pointer corruption, disabling while
   this is thorougly investigated (I see another refactor coming up) */
#define SI4735_NOI2C

#if defined(ARDUINO) && ARDUINO >= 100
# include <Arduino.h>  
#else
# include <WProgram.h>  
#endif

//Assign the default radio pin numbers (shield version)
#define SI4735_PIN_POWER 8
#define SI4735_PIN_RESET 9
#define SI4735_PIN_GPO2 2

//List of possible interfaces for Si4735
#define SI4735_INTERFACE_SPI 0
#define SI4735_INTERFACE_I2C 1

//Assign the SPI pin numbers (shield version)
//SDIO, GPO1 and SCLK always connected to MOSI, MISO and SCK for SPI mode
//SDIO and SCLK always connected to SDA and SCL for I2C mode
//SEN should be hardwired HIGH or LOW for I2C mode
#define SI4735_PIN_SEN SS

//Constants for hardwired pins
//Chip is always powered, do not use PIN_POWER semantics
#define SI4735_PIN_POWER_HW 0xFF
//Chip interfaced via I2C, GPO2 left floating or tied LOW
//Define both HW and HWL constants to suit both semantics when typing
#define SI4735_PIN_GPO2_HW 0xFF
#define SI4735_PIN_GPO2_HWL 0xFF
//Chip interfaced via I2C, SEN always HIGH, device address 0xC6
#define SI4735_PIN_SEN_HWH 0xFF
//Chip interfaced via I2C, SEN always LOW, device address 0x22
#define SI4735_PIN_SEN_HWL 0xFE

//List of possible modes for the Si4735 Radio
#define SI4735_MODE_LW 0
#define SI4735_MODE_AM 1
#define SI4735_MODE_SW 2
#define SI4735_MODE_FM 3

//Define the Locale options
#define SI4735_LOCALE_US 0
#define SI4735_LOCALE_EU 1

//Define Si4735 Command codes
#define SI4735_CMD_POWER_UP 0x01
#define SI4735_CMD_GET_REV 0x10
#define SI4735_CMD_POWER_DOWN 0x11
#define SI4735_CMD_SET_PROPERTY 0x12
#define SI4735_CMD_GET_PROPERTY 0x13
#define SI4735_CMD_GET_INT_STATUS 0x14
#define SI4735_CMD_PATCH_ARGS 0x15
#define SI4735_CMD_PATCH_DATA 0x16
#define SI4735_CMD_FM_TUNE_FREQ 0x20
#define SI4735_CMD_FM_SEEK_START 0x21
#define SI4735_CMD_FM_TUNE_STATUS 0x22
#define SI4735_CMD_FM_RSQ_STATUS 0x23
#define SI4735_CMD_FM_RDS_STATUS 0x24
#define SI4735_CMD_FM_AGC_STATUS 0x27
#define SI4735_CMD_FM_AGC_OVERRIDE 0x28
#define SI4735_CMD_AM_TUNE_FREQ 0x40
#define SI4735_CMD_AM_SEEK_START 0x41
#define SI4735_CMD_AM_TUNE_STATUS 0x42
#define SI4735_CMD_AM_RSQ_STATUS 0x43
#define SI4735_CMD_AM_AGC_STATUS 0x47
#define SI4735_CMD_AM_AGC_OVERRIDE 0x48
#define SI4735_CMD_AUX_ASRC_START 0x61
#define SI4735_CMD_AUX_ASQ_STATUS 0x65
#define SI4735_CMD_GPIO_CTL 0x80
#define SI4735_CMD_GPIO_SET 0x81

//Define Si4735 Command flags (bits fed to the chip)
#define SI4735_FLG_CTSIEN 0x80
//Renamed to GPO2IEN from GPO2OEN in datasheet to avoid conflict with real
//GPO2OEN below. Also makes more sense this way: GPO2IEN -> enable GPO2 as INT
//GPO2OEN -> enable GPO2 generically, as an output
#define SI4735_FLG_GPO2IEN 0x40
#define SI4735_FLG_PATCH 0x20
#define SI4735_FLG_XOSCEN 0x10
#define SI4735_FLG_FREEZE 0x02
#define SI4735_FLG_FAST 0x01
#define SI4735_FLG_SEEKUP 0x08
#define SI4735_FLG_WRAP 0x04
#define SI4735_FLG_CANCEL 0x02
#define SI4735_FLG_INTACK 0x01
#define SI4735_FLG_STATUSONLY 0x04
#define SI4735_FLG_MTFIFO 0x02
#define SI4735_FLG_GPO3OEN 0x08
#define SI4735_FLG_GPO2OEN 0x04
#define SI4735_FLG_GPO1OEN 0x02
#define SI4735_FLG_GPO3LEVEL 0x08
#define SI4735_FLG_GPO2LEVEL 0x04
#define SI4735_FLG_GPO1LEVEL 0x02
#define SI4735_FLG_BLETHA_0 0x00
#define SI4735_FLG_BLETHA_12 0x40
#define SI4735_FLG_BLETHA_35 0x80
#define SI4735_FLG_BLETHA_U (SI4735_FLG_BLETHA_12 | SI4735_FLG_BLETHA_35)
#define SI4735_FLG_BLETHB_0 SI4735_FLG_BLETHA_0
#define SI4735_FLG_BLETHB_12 0x10
#define SI4735_FLG_BLETHB_35 0x20
#define SI4735_FLG_BLETHB_U (SI4735_FLG_BLETHB_12 | SI4735_FLG_BLETHB_35)
#define SI4735_FLG_BLETHC_0 SI4735_FLG_BLETHA_0
#define SI4735_FLG_BLETHC_12 0x04
#define SI4735_FLG_BLETHC_35 0x08
#define SI4735_FLG_BLETHC_U (SI4735_FLG_BLETHC_12 | SI4735_FLG_BLETHC_35)
#define SI4735_FLG_BLETHD_0 SI4735_FLG_BLETHA_0
#define SI4735_FLG_BLETHD_12 0x01
#define SI4735_FLG_BLETHD_35 0x02
#define SI4735_FLG_BLETHD_U (SI4735_FLG_BLETHD_12 | SI4735_FLG_BLETHD_35)
#define SI4735_FLG_RDSEN 0x01
#define SI4735_FLG_DEEMPH_NONE 0x00
#define SI4735_FLG_DEEMPH_50 0x01
#define SI4735_FLG_DEEMPH_75 0x02
#define SI4735_FLG_RSQREP 0x08
#define SI4735_FLG_RDSREP 0x04
#define SI4735_FLG_STCREP 0x01
#define SI4735_FLG_ERRIEN 0x40
#define SI4735_FLG_RSQIEN 0x08
#define SI4735_FLG_RDSIEN 0x04
#define SI4735_FLG_STCIEN 0x01
#define SI4735_FLG_RDSNEWBLOCKB 0x20
#define SI4735_FLG_RDSNEWBLOCKA 0x10
#define SI4735_FLG_RDSSYNCFOUND 0x04
#define SI4735_FLG_RDSSYNCLOST 0x02
#define SI4735_FLG_RDSRECV 0x01

//Define Si4735 Function modes
#define SI4735_FUNC_FM 0x00
#define SI4735_FUNC_AM 0x01
#define SI4735_FUNC_VER 0x0F

//Define Si4735 Output modes
#define SI4735_OUT_RDS 0x00 // RDS only
#define SI4735_OUT_ANALOG 0x05
#define SI4735_OUT_DIGITAL1 0x0B // DCLK, LOUT/DFS, ROUT/DIO
#define SI4735_OUT_DIGITAL2 0xB0 // DCLK, DFS, DIO
#define SI4735_OUT_BOTH (SI4735_OUT_ANALOG | SI4735_OUT_DIGITAL2)

//Define Si4735 Status flag masks (bits the chip fed us)
#define SI4735_STATUS_CTS 0x80
#define SI4735_STATUS_ERR 0x40
#define SI4735_STATUS_RSQINT 0x08
#define SI4735_STATUS_RDSINT 0x04
#define SI4735_STATUS_STCINT 0x01
#define SI4735_STATUS_BLTF 0x80
#define SI4735_STATUS_AFCRL 0x02
#define SI4735_STATUS_VALID 0x01
#define SI4735_STATUS_BLENDINT 0x80
#define SI4735_STATUS_MULTHINT 0x20
#define SI4735_STATUS_MULTLINT 0x10
#define SI4735_STATUS_SNRHINT 0x08
#define SI4735_STATUS_SNRLINT 0x04
#define SI4735_STATUS_RSSIHINT 0x02
#define SI4735_STATUS_RSSILINT 0x01
#define SI4735_STATUS_SMUTE 0x08
#define SI4735_STATUS_PILOT 0x80

//Define Si4735 Property codes
#define SI4735_PROP_GPO_IEN word(0x0001)
#define SI4735_PROP_REFCLK_FREQ 0x0201
#define SI4735_PROP_REFCLK_PRESCALE 0x0202
#define SI4735_PROP_FM_DEEMPHASIS 0x1100
#define SI4735_PROP_FM_CHANNEL_FILTER 0x1102
#define SI4735_PROP_FM_BLEND_STEREO_THRESHOLD 0x1105
#define SI4735_PROP_FM_BLEND_MONO_THRESHOLD 0x1106
#define SI4735_PROP_FM_MAX_TUNE_ERROR 0x1108
#define SI4735_PROP_FM_RSQ_INT_SOURCE 0x1200
#define SI4735_PROP_FM_RSQ_SNR_HI_THRESHOLD 0x1201
#define SI4735_PROP_FM_RSQ_SNR_LO_THRESHOLD 0x1202
#define SI4735_PROP_FM_RSQ_RSSI_HI_THRESHOLD 0x1203
#define SI4735_PROP_FM_RSQ_RSSI_LO_THRESHOLD 0x1204
#define SI4735_PROP_FM_RSQ_MULTIPATH_HI_THRESHOLD 0x1205
#define SI4735_PROP_FM_RSQ_MULTIPATH_LO_THRESHOLD 0x1206
#define SI4735_PROP_FM_RSQ_BLEND_THRESHOLD 0x1207
#define SI4735_PROP_FM_SOFT_MUTE_RATE 0x1300
#define SI4735_PROP_FM_SOFT_MUTE_SLOPE 0x1301
#define SI4735_PROP_FM_SOFT_MUTE_MAX_ATTENUATION 0x1302
#define SI4735_PROP_FM_SOFT_MUTE_SNR_THRESHOLD 0x1303
#define SI4735_PROP_FM_SOFT_MUTE_RELEASE_RATE 0x1304
#define SI4735_PROP_FM_SOFT_MUTE_ATTACK_RATE 0x1305
#define SI4735_PROP_FM_SEEK_BAND_BOTTOM 0x1400
#define SI4735_PROP_FM_SEEK_BAND_TOP 0x1401
#define SI4735_PROP_FM_SEEK_FREQ_SPACING 0x1402
#define SI4735_PROP_FM_SEEK_TUNE_SNR_THRESHOLD 0x1403
#define SI4735_PROP_FM_SEEK_TUNE_RSSI_THRESHOLD 0x1404
#define SI4735_PROP_FM_RDS_INT_SOURCE 0x1500
#define SI4735_PROP_FM_RDS_INT_FIFO_COUNT 0x1501
#define SI4735_PROP_FM_RDS_CONFIG 0x1502
#define SI4735_PROP_FM_RDS_CONFIDENCE 0x1503
#define SI4735_PROP_FM_BLEND_RSSI_STEREO_THRESHOLD 0x1800
#define SI4735_PROP_FM_BLEND_RSSI_MONO_THRESHOLD 0x1801
#define SI4735_PROP_FM_BLEND_RSSI_ATTACK_RATE 0x1802
#define SI4735_PROP_FM_BLEND_RSSI_RELEASE_RATE 0x1803
#define SI4735_PROP_FM_BLEND_SNR_STEREO_THRESHOLD 0x1804
#define SI4735_PROP_FM_BLEND_SNR_MONO_THRESHOLD 0x1805
#define SI4735_PROP_FM_BLEND_SNR_ATTACK_RATE 0x1806
#define SI4735_PROP_FM_BLEND_SNR_RELEASE_RATE 0x1807
#define SI4735_PROP_FM_BLEND_MULTIPATH_STEREO_THRESHOLD 0x1808
#define SI4735_PROP_FM_BLEND_MULTIPATH_MONO_THRESHOLD 0x1809
#define SI4735_PROP_FM_BLEND_MULTIPATH_ATTACK_RATE 0x180A
#define SI4735_PROP_FM_BLEND_MULTIPATH_RELEASE_RATE 0x180B
#define SI4735_PROP_FM_HICUT_SNR_HIGH_THRESHOLD 0x1A00
#define SI4735_PROP_FM_HICUT_SNR_LOW_THRESHOLD 0x1A01
#define SI4735_PROP_FM_HICUT_ATTACK_RATE 0x1A02
#define SI4735_PROP_FM_HICUT_RELEASE_RATE 0x1A03
#define SI4735_PROP_FM_HICUT_MULTIPATH_TRIGGER_THRESHOLD 0x1A04
#define SI4735_PROP_FM_HICUT_MULTIPATH_END_THRESHOLD 0x1A05
#define SI4735_PROP_FM_HICUT_CUTOFF_FREQUENCY 0x1A06
#define SI4735_PROP_AM_DEEMPHASIS 0x3100
#define SI4735_PROP_AM_CHANNEL_FILTER 0x3102
#define SI4735_PROP_AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN 0x3103
#define SI4735_PROP_AM_MODE_AFC_SW_PULL_IN_RANGE 0x3104
#define SI4735_PROP_AM_MODE_AFC_SW_LOCK_IN_RANGE 0x3105
#define SI4735_PROP_AM_RSQ_INTERRUPTS 0x3200
#define SI4735_PROP_AM_RSQ_SNR_HIGH_THRESHOLD 0x3201
#define SI4735_PROP_AM_RSQ_SNR_LOW_THRESHOLD 0x3202
#define SI4735_PROP_AM_RSQ_RSSI_HIGH_THRESHOLD 0x3203
#define SI4735_PROP_AM_RSQ_RSSI_LOW_THRESHOLD 0x3204
#define SI4735_PROP_AM_SOFT_MUTE_RATE 0x3300
#define SI4735_PROP_AM_SOFT_MUTE_SLOPE 0x3301
#define SI4735_PROP_AM_SOFT_MUTE_MAX_ATTENUATION 0x3302
#define SI4735_PROP_AM_SOFT_MUTE_SNR_THRESHOLD 0x3303
#define SI4735_PROP_AM_SOFT_MUTE_RELEASE_RATE 0x3304
#define SI4735_PROP_AM_SOFT_MUTE_ATTACK_RATE 0x3305
#define SI4735_PROP_AM_SEEK_BAND_BOTTOM 0x3400
#define SI4735_PROP_AM_SEEK_BAND_TOP 0x3401
#define SI4735_PROP_AM_SEEK_FREQ_SPACING 0x3402
#define SI4735_PROP_AM_SEEK_TUNE_SNR_THRESHOLD 0x3403
#define SI4735_PROP_AM_SEEK_TUNE_RSSI_THRESHOLD 0x3404
#define SI4735_PROP_RX_VOLUME 0x4000
#define SI4735_PROP_RX_HARD_MUTE 0x4001

//Define RDS-related public flags (that may come handy to the user)
#define SI4735_RDS_DI_STEREO 0x01
#define SI4735_RDS_DI_ARTIFICIAL_HEAD 0x02
#define SI4735_RDS_DI_COMPRESSED 0x04
#define SI4735_RDS_DI_DYNAMIC_PTY 0x08

//This holds the current station reception metrics as given by the chip. See
//the Si4735 datasheet for a detailed explanation of each member.
typedef struct {
    byte STBLEND;
    bool PILOT;
    byte RSSI;
    byte SNR;
    byte MULT;
    signed char FREQOFF;
} Si4735_RX_Metrics;

//This holds time of day as received via RDS. Mimicking struct tm from
//<time.h> for familiarity.
//NOTE: RDS does not provide seconds, only guarantees that the minute update
//      will occur close enough to the seconds going from 59 to 00 to be
//      meaningful -- so we don't provide tm_sec
//NOTE: RDS does not provide DST information so we don't provide tm_isdst
//NOTE: we will provide tm_wday (day of week) but not tm_yday (day of year)
typedef struct {
    byte tm_min;
    byte tm_hour;
    byte tm_mday;
    byte tm_mon;
    word tm_year;
    byte tm_wday;
}  Si4735_RDS_Time;

typedef struct {
    //PI is already taken :-(
    word programIdentifier;
    bool TP, TA, MS;
    byte PTY, DICC;    
    char programService[9];
    char programTypeName[9];
    char radioText[65];
} Si4735_RDS_Data;

class Si4735RDSDecoder
{
    public:
        /*
        * Description:
        *   Default constructor.
        */
        Si4735RDSDecoder() { resetRDS(); }
        
        /*
        * Description:
        *   Decodes one RDS block and updates internal data structures.
        */
        void decodeRDSBlock(word block[]);

        /*
        * Description:
        *   Returns currently decoded RDS data, filling a struct 
        *   Si4735_RDS_Data.
        */
        void getRDSData(Si4735_RDS_Data* rdsdata);

        /*
        * Description:
        *   Returns currently decoded RDS CT information filling a struct
        *   Si4735_RDS_Time, if any is available, and returns true; otherwise
        *   returns false and does not touch rdstime.
        * Parameters:
        *   rdstime - pointer to a struct Si4735_RDS_Time to be filled with
        *             CT information, ignore if only interested in CT
        *             availability and not actual value.
        */
        bool getRDSTime(Si4735_RDS_Time* rdstime = NULL);
        
        /*
        * Description:
        *   Resets internal data structures, use when switching to a new
        *   station.
        */
        void resetRDS(void);
        
#if defined(SI4735_DEBUG)
        /* Description:
        *    Dumps RDS group type receipt statistics.
        */
        void dumpRDSStats(void);
#endif
        
    private:
        Si4735_RDS_Data _status;
        Si4735_RDS_Time _time;
        bool _rdstextab, _rdsptynab, _havect;
#if defined(SI4735_DEBUG)
        word _rdsstats[32];
#endif
        /*
        * Description:
        *   Filters the string str in place to only contain printable 
        *   characters and also replaces 0x0D (CR) with 0x00 effectively
        *   ending the string at that point as per RDBS §3.1.5.3.
        *   Any unprintable character is converted to a question mark ("?"),
        *   as is customary. This helps with filtering out noisy strings.
        */
        void makePrintable(char* str);

        /*
        * Description:
        *   Switches endianness of the given value around. Si4735 is a 
        *   big-endian machine while Arduino is little-endian --  a storm of
        *   problems are headed our way if we ignore that.
        * Parameters:
        *   value - the word to be switched
        */
        inline word switchEndian(word value) { return (value >> 8) | (value << 8); }
};

class Si4735Translate
{
    public:
        /*
        * Description:
        *   Translates the given PTY into human-readable text for the given
        *   locale. At most textsize-1 characters will be copied to the buffer
        *   at text.
        */
        void getTextForPTY(byte PTY, byte locale, char* text, byte textsize);
        
        /*
        * Description:
        *   Translates the given PTY between the given locales.
        */
        byte translatePTY(byte PTY, byte fromlocale, byte tolocale);
        
        /*
        * Description:
        *   Decodes the station callsign out of the PI using the method
        *   defined in the RDBS standard for North America.
        * Parameters:
        *   programIdentifier - a word containing the Program Identifier value
        *                       from RDS
        *   callSign - pointer to a char[] at least 5 characters long that 
        *              receives the decoded station call sign
        */
        void decodeCallSign(word programIdentifier, char* callSign);
};

class Si4735
{
    public:
        /*
        * Description:
        *   This the constructor. It assumes SparkFun Si4735 Shield + level
        *   shifter (or the diode fix, or 3.3V-native Arduino) semantics if
        *   called without parameters.
        *   If you're not using the Shield (e.g. using the Breakout Board) or
        *   have wired the Si4735 differently, then explicitly supply the
        *   constructor with the actual pin numbers.
        *   Use the hardwired pins constants above to tell the constructor you
        *   haven't used (and hardwired) some of the pins.
        * Parameters:
        *   interface - interface and protocol used to talk to the chip 
        *   pin*      - pin numbers for connections to the Si4735, with
        *               defaults for the SparkFun Si4735 Shield already
        *               provided.
        */
        Si4735(byte interface = SI4735_INTERFACE_SPI, 
               byte pinPower = SI4735_PIN_POWER,
               byte pinReset = SI4735_PIN_RESET,
               byte pinGPO2 = SI4735_PIN_GPO2, byte pinSEN = SI4735_PIN_SEN);
        
        /*
        * Description:
        *   This is the destructor, it shuts the Si4735 down
        */
        ~Si4735() { end(true); };
        
        /*
        * Description: 
        *   Initializes the Si4735, powers up the radio in the desired mode 
        *   and limits the bandwidth appropriately.
        *   This function must be called before any other radio command.
        *   The band limits are set as follows:
        *     LW - 152 to 279 kHz
        *     AM - 520 to 1710 kHz
        *     SW - 2.3 to 23 MHz
        *     FM - 87.5 to 107.9 MHz
        * Parameters:
        *   mode        - The desired radio mode, one of the SI4735_MODE_*
        *                 constants.
        *   xosc        - A 32768Hz external oscillator is present.
        *   slowshifter - A BOB-08745 is used for level shifting between an
        *                 Uno/Mega and the Si4735. Use a 3.3V I/O Arduino or
        *                 shift through a BOB-10403 to be able to go up to
        *                 1MHz by setting this to false.
        */
        void begin(byte mode, bool xosc = true, bool slowshifter = true);
        
        /*
        * Description: 
        *   Used to send a command and its arguments to the radio chip.
        * Parameters:
        *   command - the command byte, see datasheet and use one of the
                      SI4735_CMD_* constants
        *   arg1-7  - command arguments, see the Si4735 Programmers Guide.
        */
        void sendCommand(byte command, byte arg1 = 0, byte arg2 = 0,
                         byte arg3 = 0, byte arg4 = 0, byte arg5 = 0,
                         byte arg6 = 0, byte arg7 = 0);

        /*
        * Description: 
        *   Acquires certain revision parameters from the Si4735 chip, returns
        *   the last two digits of the chip part number (e.g. 35 for Si4735).
        * Parameters:
        *   FW  - Firmware Version and it is a 2 character string
        *   CMP - Component Revision and it is a 2 character string
        *   REV - Chip Die Revision and it is a single character
        *   Set to NULL (or ignore) whichever parameter you don't care about.
        * Chips are usually referred in datasheets as "Si4735-$REV$FW", for
        * example "Si4735-C40" for the chip on the Sparkfun Shield.
        */
        byte getRevision(char* FW = NULL, char* CMP = NULL, char* REV = NULL,
                         word* patch = NULL);

        /*
        * Description: 
        *   Used to to tune the radio to a desired frequency. The library uses
        *   the mode indicated via begin() to determine how to set the 
        *   frequency.
        * Parameters:
        *   frequency - The frequency to tune to, in kHz (or in 10kHz if using
        *               FM mode).
        */
        void setFrequency(word frequency);

        /*
        * Description:
        *   Gets the frequency the chip is currently tuned to.    
        * Parameters:
        *   valid - will be set to true if the chip currently detects a valid
        *           (as defined by current Seek/Tune criteria, see
        *           FM_SEEK_TUNE_* properties in the datasheet) signal on this
        *           frequency. Omit if you don't care.
        */
        word getFrequency(bool* valid = NULL);

        /*
        * Description:
        *   Commands the radio to seek up to the next valid channel.
        * Parameters:
        *   wrap - set to true to allow the seek to wrap around the current
        *          band.
        */
        void seekUp(bool wrap = true);

        /*
        * Description:
        *   Commands the radio to seek down to the next valid channel.
        * Parameters:
        *   wrap - set to true to allow the seek to wrap around the current
        *          band.
        */
        void seekDown(bool wrap = true);
        
        /*
        * Description:
        *   Adjust the threshold levels of the seek function.
        *   FM Ranges:
        *     SNR = [0-127] in dB, default = 3dB
        *     RSSI = [0-127] in dBuV, default = 20dBuV
        *   AM Ranges:
        *     SNR = [0-63] in dB, default = 5dB
        *     RSSI = [0-63] in dBuV, default = 19dBuV
        */    
        void setSeekThresholds(byte SNR, byte RSSI);

        /*
        * Description:
        *   If in FM mode and the chip has received any RDS block, fetch it
        *   off the chip and fill word block[4] with it, returning true;
        *   otherwise return false without side-effects.
        *   This function needs to be actively called (e.g. from loop()) in
        *   order to see sensible information.
        */
        bool readRDSBlock(word* block);

        /*
        * Description:
        *   Returns true if at least one RDS group has been received while
        *   tuned into the current station.
        */
        bool isRDSCapable(void) { return _haverds; };

        /*
        * Description:
        *   Retrieves the Received Signal Quality metrics using a 
        *   Si4735_RX_Metrics struct.
        */
        void getRSQ(Si4735_RX_Metrics* RSQ);

        /*
        * Description:
        *   Sets the volume. Valid values are [0-63]. 
        */
        void setVolume(byte value) {
            setProperty(SI4735_PROP_RX_VOLUME, 
                        word(0x00, constrain(value, 0, 63)));
        };

        /*
        * Description:
        *   Gets the current volume.
        */
        byte getVolume(void) {
            return lowByte(getProperty(SI4735_PROP_RX_VOLUME));
        };

        /*
        * Description:
        *   Increase the volume by 1. If the maximum volume has been 
        *   reached, no further increase will take place and returns false;
        *   otherwise true.
        */
        bool volumeUp(void);
        
        /*
        * Description:
        *   Decrease the volume by 1. If the minimum volume has been 
        *   reached, no further decrease will take place and returns false;
        *   otherwise true.
        * Parameters:
        *   alsomute - mute the output when reaching minimum volume, in 
        *               addition to returning false
        */
        bool volumeDown(bool alsomute = false);
        
        /*
        * Description:
        *   Mutes the audio output.
        */
        void mute(void) {
            setProperty(SI4735_PROP_RX_HARD_MUTE, word(0x00, 0x03));
        };

        /*
        * Description:
        *   Unmutes the audio output.
        * Parameters:
        *   minvol - set the volume to minimum value before unmuting if true,
        *            otherwise leave it untouched causing the chip to blast
        *            audio out at whatever the previous volume level was.
        */
        void unMute(bool minvol = false);

        /*
        * Description:
        *   Gets the current status (short read) of the radio. Learn more
        *   about the status byte in the Si4735 Datasheet.
        * Returns:
        *   The status of the radio.
        */
        byte getStatus(void);

        /*
        * Description:
        *   Gets the long response (long read) from the radio. Learn more
        *   about the long response in the Si4735 Datasheet.
        * Parameters:
        *   response - A byte[] at least 16 bytes long for the response from
        *              the radio to be stored in.
        */
        void getResponse(byte* response);

        /*
        * Description:
        *   Powers down the radio.
        * Parameters:
        *   hardoff - physically power down the chip if fed off a digital pin,
        *             otherwise just send SI4735_CMD_POWER_DOWN.
        */
        void end(bool hardoff = false);

        /*
        * Description:
        *   Sets deemphasis time constant (see SI4735_FLG_DEEMPH_*).
        */
        void setDeemphasis(byte deemph);

        /*
        * Description:
        *   Gets the current mode of the radio (see SI4735_MODE_*).
        */
        byte getMode(void) { return _mode; };

        /*
        * Description:
        *   Sets the Mode of the radio.
        * Parameters:
        *   mode      - the new mode of operation (see SI4735_MODE_*).
        *   powerdown - power the chip down first, as required by datasheet.
        *   xosc      - an external 32768Hz oscillator is present.
        */
        void setMode(byte mode, bool powerdown = true, 
                     bool xosc = true);

        /*
        * Description:
        *   Sets a property value, see the SI4735_PROP_* constants and the
        *   Si4735 Datasheet for more information.
        */
        void setProperty(word property, word value);
        
        /*
        * Description:
        *   Gets a property value, see the SI4735_PROP_* constants and the
        *   Si4735 Datasheet for more information.
        * Returns:
        *   The current value of property.
        */
        word getProperty(word property);

    private:
        byte _pinPower, _pinReset, _pinGPO2, _pinSDIO, _pinGPO1, _pinSCLK,
             _pinSEN;
        byte _mode, _response[16], _i2caddr;
        bool _haverds;
        
        /*
        * Description:
        *   Enables RDS reception.
        */
        void enableRDS(void);
        
        /*
        * Description:
        *   Waits for completion of various operations.
        * Parameters:
        *   which - interrupt flag to wait for, see SI4735_STATUS_*
        */
        void waitForInterrupt(byte which);
        
        /*
        * Description:
        *   Performs actions common to all tuning modes.
        */
        void completeTune(void);
};

#endif
