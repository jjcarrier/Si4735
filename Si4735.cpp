/* Arduino Si4735 Library
 * See the README file for author and licensing information. In case it's
 * missing from your distribution, use the one here as the authoritative
 * version: https://github.com/csdexter/Si4735/blob/master/README
 *
 * This library is for use with the SparkFun Si4735 Shield or Breakout Board.
 * See the example sketches to learn how to use the library in your code.
 *
 * This is the main code file for the library.
 * See the header file for better function documentation.
 */

#include "Si4735.h"
#include "Si4735-private.h"

#if !defined(SI4735_NOSPI)
# include <SPI.h>
#endif
#if !defined(SI4735_NOI2C)
# include <Wire.h>
#endif

void Si4735RDSDecoder::decodeRDSBlock(word block[]){
    byte grouptype;
    word fourchars[2];

    _status.programIdentifier = block[0];
    grouptype = lowByte((block[1] & SI4735_RDS_TYPE_MASK) >>
                        SI4735_RDS_TYPE_SHR);
    _status.TP = block[1] & SI4735_RDS_TP;
    _status.PTY = lowByte((block[1] & SI4735_RDS_PTY_MASK) >>
                          SI4735_RDS_PTY_SHR);
#if defined(SI4735_DEBUG)
    _rdsstats[grouptype]++;
#endif

    switch(grouptype){
        case SI4735_GROUP_0A:
        case SI4735_GROUP_0B:
        case SI4735_GROUP_15B:
            byte DIPSA;
            word twochars;

            _status.TA = block[1] & SI4735_RDS_TA;
            _status.MS = block[1] & SI4735_RDS_MS;
            DIPSA = lowByte(block[1] & SI4735_RDS_DIPS_ADDRESS);
            bitWrite(_status.DICC, 3 - DIPSA, block[1] & SI4735_RDS_DI);
            twochars = switchEndian(block[3]);
            strncpy(&_status.programService[DIPSA * 2], (char *)&twochars, 2);
            if(grouptype == SI4735_GROUP_0A) {
                //TODO: read the standard and do AF list decoding
            }
            break;
        case SI4735_GROUP_1A:
        case SI4735_GROUP_1B:
            //TODO: read the standard and do PIN and slow labeling codes
            break;
        case SI4735_GROUP_2A:
        case SI4735_GROUP_2B:
            byte RTA, RTAW;

            if((block[1] & SI4735_RDS_TEXTAB) != _rdstextab) {
                _rdstextab = !_rdstextab;
                memset(_status.radioText, ' ', 64);
            }
            RTA = lowByte(block[1] & SI4735_RDS_TEXT_ADDRESS);
            RTAW = (grouptype == SI4735_GROUP_2A) ? 4 : 2;
            fourchars[0] = switchEndian(
                block[(grouptype == SI4735_GROUP_2A) ? 2 : 3]);
            if(grouptype == SI4735_GROUP_2A) 
                fourchars[1] = switchEndian(block[3]);
            strncpy(&_status.radioText[RTA * RTAW], (char *)fourchars, RTAW);
            break;
        case SI4735_GROUP_3A:
            //TODO: read the standard and do AID listing
            break;
        case SI4735_GROUP_3B:
        case SI4735_GROUP_4B:
        case SI4735_GROUP_6A:
        case SI4735_GROUP_6B:
        case SI4735_GROUP_7B:
        case SI4735_GROUP_8B:
        case SI4735_GROUP_9B:
        case SI4735_GROUP_10B:
        case SI4735_GROUP_11A:
        case SI4735_GROUP_11B:
        case SI4735_GROUP_12A:
        case SI4735_GROUP_12B:
        case SI4735_GROUP_13B:
            //Application data payload (ODA), ignore for now
            break;
        case SI4735_GROUP_4A:
            unsigned long MJD, CT, ys;
            word yp;
            byte k, mp;

            CT = ((unsigned long)block[2] << 16) | block[3];
            //The standard mandates that CT must be all zeros if no time
            //information is being provided by the current station.
            if(!CT) break;

            _havect = true;            
            MJD = (unsigned long)(block[1] & SI4735_RDS_MJD_MASK) <<
                  SI4735_RDS_MJD_SHL;
            MJD |= (CT & SI4735_RDS_TIME_MJD_MASK) >> SI4735_RDS_TIME_MJD_SHR;

            //We report UTC for now, better not fiddle with timezones
            _time.tm_hour = (CT & SI4735_RDS_TIME_HOUR_MASK) >>
                                          SI4735_RDS_TIME_HOUR_SHR;
            _time.tm_min = (CT & SI4735_RDS_TIME_MINUTE_MASK) >>
                                         SI4735_RDS_TIME_MINUTE_SHR;
            //Use integer arithmetic at all costs, Arduino lacks an FPU
            yp = (MJD * 10 - 150782) * 10 / 36525;
            ys = yp * 36525 / 100;
            mp = (MJD * 10 - 149561 - ys * 10) * 1000 / 306001;
            _time.tm_mday = MJD - 14956 - ys - mp * 306001 / 10000;
            k = (mp == 14 || mp == 15) ? 1 : 0;
            _time.tm_year = 1900 + yp + k;
            _time.tm_mon = mp - 1 - k * 12;
            _time.tm_wday = (MJD + 2) % 7 + 1;
            break;
        case SI4735_GROUP_5A:
        case SI4735_GROUP_5B:
            //TODO: read the standard and do TDC listing
            break;
        case SI4735_GROUP_7A:
            //TODO: read the standard and do Radio Paging
            break;
        case SI4735_GROUP_8A:
            //TODO: read the standard and do TMC listing
            break;
        case SI4735_GROUP_9A:
            //TODO: read the standard and do EWS listing
            break;
        case SI4735_GROUP_10A:
            if((block[1] & SI4735_RDS_PTYNAB) != _rdsptynab) {
                _rdsptynab = !_rdsptynab;
                memset(_status.programTypeName, ' ', 8);
            }
            fourchars[0] = switchEndian(block[2]);
            fourchars[1] = switchEndian(block[3]);
            strncpy(&_status.programTypeName[(block[1] & 
                                              SI4735_RDS_PTYN_ADDRESS) * 4], 
                    (char *)&fourchars, 4);            
            break;
        case SI4735_GROUP_13A:
            //TODO: read the standard and do Enhanced Radio Paging
            break;
        case SI4735_GROUP_14A:
        case SI4735_GROUP_14B:
            //TODO: read the standard and do EON listing
            break;
        case SI4735_GROUP_15A:
            //Withdrawn and currently unallocated, ignore
            break;
    }
}

void Si4735RDSDecoder::getRDSData(Si4735_RDS_Data* rdsdata){
    makePrintable(_status.programService);
    makePrintable(_status.programTypeName);
    makePrintable(_status.radioText);

    *rdsdata = _status;
}

bool Si4735RDSDecoder::getRDSTime(Si4735_RDS_Time* rdstime){
    if(_havect && rdstime) *rdstime = _time;

    return _havect;
}

void Si4735RDSDecoder::resetRDS(void){
    memset(_status.programService, ' ', 8);
    _status.programService[8] = '\0';
    memset(_status.programTypeName, ' ', 8);
    _status.programTypeName[8] = '\0';
    memset(_status.radioText, ' ', 64);
    _status.radioText[64] = '\0';    
    _status.DICC = 0;
    _rdstextab = false;
    _rdsptynab = false;
    _havect = false;
#if defined(SI4735_DEBUG)
    memset((void *)&_rdsstats, 0x00, sizeof(_rdsstats));
#endif
}

void Si4735RDSDecoder::makePrintable(char* str){
    for(byte i = 0; i < strlen(str); i++) {
        if(str[i] == 0x0D) {
            str[i] = '\0';
            break;
        }
        if(str[i] < 32 || str[i] > 126) str[i] = '?';
    }
}

#if defined(SI4735_DEBUG)
void Si4735RDSDecoder::dumpRDSStats(void){
    Serial.println("RDS group statistics:");
    for(byte i = 0; i < 32; i++) {
        Serial.print("#");
        Serial.print(i >> 1);
        Serial.print((i & 0x01) ? 'B' : 'A');
        Serial.print(": ");
        Serial.println(_rdsstats[i]);
    }
    Serial.flush();
}
#endif

const char Si4735_PTY2Text_S_None[] PROGMEM = "None/Undefined";
const char Si4735_PTY2Text_S_News[] PROGMEM = "News";
const char Si4735_PTY2Text_S_Current[] PROGMEM = "Current affairs";
const char Si4735_PTY2Text_S_Information[] PROGMEM = "Information";
const char Si4735_PTY2Text_S_Sports[] PROGMEM = "Sports";
const char Si4735_PTY2Text_S_Education[] PROGMEM = "Education";
const char Si4735_PTY2Text_S_Drama[] PROGMEM = "Drama";
const char Si4735_PTY2Text_S_Culture[] PROGMEM = "Culture";
const char Si4735_PTY2Text_S_Science[] PROGMEM = "Science";
const char Si4735_PTY2Text_S_Varied[] PROGMEM = "Varied";
const char Si4735_PTY2Text_S_Pop[] PROGMEM = "Pop";
const char Si4735_PTY2Text_S_Rock[] PROGMEM = "Rock";
const char Si4735_PTY2Text_S_EasySoft[] PROGMEM = "Easy & soft";
const char Si4735_PTY2Text_S_Classical[] PROGMEM = "Classical";
const char Si4735_PTY2Text_S_Other[] PROGMEM = "Other music";
const char Si4735_PTY2Text_S_Weather[] PROGMEM = "Weather";
const char Si4735_PTY2Text_S_Finance[] PROGMEM = "Finance";
const char Si4735_PTY2Text_S_Children[] PROGMEM = "Children's";
const char Si4735_PTY2Text_S_Social[] PROGMEM = "Social affairs";
const char Si4735_PTY2Text_S_Religion[] PROGMEM = "Religion";
const char Si4735_PTY2Text_S_TalkPhone[] PROGMEM = "Talk & phone-in";
const char Si4735_PTY2Text_S_Travel[] PROGMEM = "Travel";
const char Si4735_PTY2Text_S_Leisure[] PROGMEM = "Leisure";
const char Si4735_PTY2Text_S_Jazz[] PROGMEM = "Jazz";
const char Si4735_PTY2Text_S_Country[] PROGMEM = "Country";
const char Si4735_PTY2Text_S_National[] PROGMEM = "National";
const char Si4735_PTY2Text_S_Oldies[] PROGMEM = "Oldies";
const char Si4735_PTY2Text_S_Folk[] PROGMEM = "Folk";
const char Si4735_PTY2Text_S_Documentary[] PROGMEM = "Documentary";
const char Si4735_PTY2Text_S_EmergencyTest[] PROGMEM = "Emergency test";
const char Si4735_PTY2Text_S_Emergency[] PROGMEM = "Emergency";
const char Si4735_PTY2Text_S_Adult[] PROGMEM = "Adult hits";
const char Si4735_PTY2Text_S_Top40[] PROGMEM = "Top 40";
const char Si4735_PTY2Text_S_Nostalgia[] PROGMEM = "Nostalgia";
const char Si4735_PTY2Text_S_RnB[] PROGMEM = "Rhythm and blues";
const char Si4735_PTY2Text_S_Language[] PROGMEM = "Language";
const char Si4735_PTY2Text_S_Personality[] PROGMEM = "Personality";
const char Si4735_PTY2Text_S_Public[] PROGMEM = "Public";
const char Si4735_PTY2Text_S_College[] PROGMEM = "College";

const char * const Si4735_PTY2Text_EU[32] PROGMEM = {
    Si4735_PTY2Text_S_None, 
    Si4735_PTY2Text_S_News,
    Si4735_PTY2Text_S_Current,
    Si4735_PTY2Text_S_Information,
    Si4735_PTY2Text_S_Sports,
    Si4735_PTY2Text_S_Education, 
    Si4735_PTY2Text_S_Drama,
    Si4735_PTY2Text_S_Culture,
    Si4735_PTY2Text_S_Science,
    Si4735_PTY2Text_S_Varied,
    Si4735_PTY2Text_S_Pop,
    Si4735_PTY2Text_S_Rock,
    Si4735_PTY2Text_S_EasySoft,
    Si4735_PTY2Text_S_Classical,
    Si4735_PTY2Text_S_Classical,
    Si4735_PTY2Text_S_Other,
    Si4735_PTY2Text_S_Weather, 
    Si4735_PTY2Text_S_Finance,
    Si4735_PTY2Text_S_Children,
    Si4735_PTY2Text_S_Social,
    Si4735_PTY2Text_S_Religion, 
    Si4735_PTY2Text_S_TalkPhone,
    Si4735_PTY2Text_S_Travel,
    Si4735_PTY2Text_S_Leisure,
    Si4735_PTY2Text_S_Jazz,
    Si4735_PTY2Text_S_Country,
    Si4735_PTY2Text_S_National,
    Si4735_PTY2Text_S_Oldies,
    Si4735_PTY2Text_S_Folk,
    Si4735_PTY2Text_S_Documentary,
    Si4735_PTY2Text_S_EmergencyTest,
    Si4735_PTY2Text_S_Emergency};

const char * const Si4735_PTY2Text_US[32] PROGMEM = {
    Si4735_PTY2Text_S_None, 
    Si4735_PTY2Text_S_News,
    Si4735_PTY2Text_S_Information,
    Si4735_PTY2Text_S_Sports,
    Si4735_PTY2Text_S_TalkPhone,
    Si4735_PTY2Text_S_Rock,
    Si4735_PTY2Text_S_Rock,
    Si4735_PTY2Text_S_Adult, 
    Si4735_PTY2Text_S_Rock,
    Si4735_PTY2Text_S_Top40,
    Si4735_PTY2Text_S_Country,
    Si4735_PTY2Text_S_Oldies,
    Si4735_PTY2Text_S_EasySoft,
    Si4735_PTY2Text_S_Nostalgia,
    Si4735_PTY2Text_S_Jazz, 
    Si4735_PTY2Text_S_Classical, 
    Si4735_PTY2Text_S_RnB,
    Si4735_PTY2Text_S_RnB,
    Si4735_PTY2Text_S_Language,
    Si4735_PTY2Text_S_Religion, 
    Si4735_PTY2Text_S_Religion,
    Si4735_PTY2Text_S_Personality,
    Si4735_PTY2Text_S_Public,
    Si4735_PTY2Text_S_College,
    Si4735_PTY2Text_S_None,
    Si4735_PTY2Text_S_None,
    Si4735_PTY2Text_S_None,
    Si4735_PTY2Text_S_None,
    Si4735_PTY2Text_S_None, 
    Si4735_PTY2Text_S_Weather,
    Si4735_PTY2Text_S_EmergencyTest, 
    Si4735_PTY2Text_S_Emergency};

const byte Si4735_PTY_EU2US[32] PROGMEM = {0, 1, 0, 2, 3, 23, 0, 0, 0, 0, 7,
                                           5, 12, 15, 15, 0, 29, 0, 0, 0, 20,
                                           4, 0, 0, 14, 10, 0, 11, 0, 0, 30,
                                           31};
const byte Si4735_PTY_US2EU[32] PROGMEM = {0, 1, 3, 4, 21, 11, 11, 10, 11, 10,
                                           25, 27, 12, 27, 24, 14, 15, 15, 0,
                                           20, 20, 0, 0, 5, 0, 0, 0, 0, 0, 16,
                                           30, 31};

void Si4735Translate::getTextForPTY(byte PTY, byte locale, char* text,
                                    byte textsize){
    switch(locale){
        case SI4735_LOCALE_US:
            strncpy_P(text, (PGM_P)(pgm_read_word(&Si4735_PTY2Text_US[PTY])),
                      textsize);
            break;
        case SI4735_LOCALE_EU:
            strncpy_P(text, (PGM_P)(pgm_read_word(&Si4735_PTY2Text_EU[PTY])),
                    textsize);
            break;
    }
}

byte Si4735Translate::translatePTY(byte PTY, byte fromlocale, byte tolocale){
    if(fromlocale == tolocale) return PTY;
    else switch(fromlocale){
        case SI4735_LOCALE_US:
            return pgm_read_byte(&Si4735_PTY_US2EU[PTY]);
            break;
        case SI4735_LOCALE_EU:
            return pgm_read_byte(&Si4735_PTY_EU2US[PTY]);
            break;
    }
    
    //Never reached
    return 0;
}

void Si4735Translate::decodeCallSign(word programIdentifier, char* callSign){
    //TODO: read the standard and implement world-wide PI decoding
    if(programIdentifier >= 21672){
        callSign[0] = 'W';
        programIdentifier -= 21672;
    } else 
        if(programIdentifier < 21672 && programIdentifier >= 0x1000){
            callSign[0] = 'K';
            programIdentifier -= 0x1000;
        } else programIdentifier -= 1;
    if(programIdentifier >= 0){
        callSign[1] = char(programIdentifier / 676 + 'A');
        callSign[2] = char((programIdentifier - 676 * programIdentifier / 
                            676) / 26 + 'A');
        callSign[3] = char(((programIdentifier - 676 * programIdentifier /
                             676) % 26 ) + 'A');
        callSign[4] = '\0';        
    } else strcpy(callSign, "UNKN");
}

Si4735::Si4735(byte interface, byte pinPower, byte pinReset, byte pinGPO2,
               byte pinSEN){
    _mode = SI4735_MODE_FM;
    _pinPower = pinPower;
    _pinReset = pinReset;
    _pinGPO2 = pinGPO2;
    _pinSEN = pinSEN;
    switch(interface){
        case SI4735_INTERFACE_SPI:
            _i2caddr = 0x00;
            break;
        case SI4735_INTERFACE_I2C:
            if(_pinSEN == SI4735_PIN_SEN_HWH) _i2caddr = SI4735_I2C_ADDR_H;
            else _i2caddr = SI4735_I2C_ADDR_L;
            break;
    }
}

void Si4735::begin(byte mode, bool xosc, bool slowshifter){
    //Start by resetting the Si4735 and configuring the communication protocol
    if(_pinPower != SI4735_PIN_POWER_HW) pinMode(_pinPower, OUTPUT);
    pinMode(_pinReset, OUTPUT);
    //GPO1 is connected to MISO on the shield, the latter of which defaults to
    //INPUT mode on boot which makes it High-Z, which, in turn, allows the
    //pull-up inside the Si4735 to work its magic.
    //For non-Shield, non SPI configurations, leave GPO1 floating or tie to 
    //HIGH.
    if(!_i2caddr) {
        //GPO2 must be driven HIGH after reset to select SPI
        pinMode(_pinGPO2, OUTPUT);
    };
    pinMode((_i2caddr ? SCL : SCK), OUTPUT);

    //Sequence the power to the Si4735
    if(_pinPower != SI4735_PIN_POWER_HW) digitalWrite(_pinPower, LOW);
    digitalWrite(_pinReset, LOW);

    if(!_i2caddr) {
        //Configure the device for SPI communication
        digitalWrite(_pinGPO2, HIGH);
    };
    //Use the longest of delays given in the datasheet
    delayMicroseconds(100);
    if(_pinPower != SI4735_PIN_POWER_HW) {
        digitalWrite(_pinPower, HIGH);
        //Datasheet calls for 250us between VIO and RESET
        delayMicroseconds(250);
    };
    digitalWrite((_i2caddr ? SCL : SCK), LOW);
    //Datasheet calls for no rising SCLK edge 300ns before RESET rising edge,
    //but Arduino can only go as low as 3us.
    delayMicroseconds(5);
    digitalWrite(_pinReset, HIGH);
    //Datasheet calls for 30ns from rising edge of RESET until GPO1/GPO2 bus
    //mode selection completes, but Arduino can only go as low as 3us.
    delayMicroseconds(5);

    if(!_i2caddr) {
        //Now configure the I/O pins properly
        pinMode(MISO, INPUT);
    };
    //If we get to here and in SPI mode, we know GPO2 is not unused because
    //we just used it to select SPI mode. If we are in I2C mode, then we look
    //to see if the user wants interrupts and only then enable it.
    if(_pinGPO2 != SI4735_PIN_GPO2_HW) pinMode(_pinGPO2, INPUT);
    
    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        //Configure the SPI hardware
        SPI.begin();
        //If SEN is NOT wired to SS, we need to manually configure it,
        //otherwise SPI.begin() above already did it for us.
        if(_pinSEN != SS) {
            pinMode(_pinSEN, OUTPUT);
            digitalWrite(_pinSEN, HIGH);
        }
        //Datahseet says Si4735 can't do more than 2.5MHz on SPI and if you're
        //level shifting through a BOB-08745, you can't do more than 250kHz 
        SPI.setClockDivider((slowshifter ? SPI_CLOCK_DIV64 : SPI_CLOCK_DIV8));
        //SCLK idle LOW, SDIO sampled on RISING edge
        SPI.setDataMode(SPI_MODE0);
        //Datasheet says Si4735 is big endian (MSB first)
        SPI.setBitOrder(MSBFIRST);
#endif
    } else {
#if !defined(SI4735_NOI2C)
        //Configure the I2C hardware
        Wire.begin();
#endif
    };

    setMode(mode, false, xosc);
}

void Si4735::sendCommand(byte command, byte arg1, byte arg2, byte arg3, 
                         byte arg4, byte arg5, byte arg6, byte arg7){
    byte status;

#if defined(SI4735_DEBUG)
    Serial.print("Si4735 CMD 0x");
    Serial.print(command, HEX);
    Serial.print(" (0x");
    Serial.print(arg1, HEX);
    Serial.print(" [");
    Serial.print(arg1, BIN);
    Serial.print("], 0x");    
    Serial.print(arg2, HEX);
    Serial.print(" [");
    Serial.print(arg2, BIN);
    Serial.print("], 0x");
    Serial.print(arg3, HEX);
    Serial.print(" [");
    Serial.print(arg3, BIN);
    Serial.println("],");
    Serial.print("0x");
    Serial.print(arg4, HEX);
    Serial.print(" [");
    Serial.print(arg4, BIN);
    Serial.print("], 0x");
    Serial.print(arg5, HEX);
    Serial.print(" [");
    Serial.print(arg5, BIN);
    Serial.print("], 0x");
    Serial.print(arg6, HEX);
    Serial.print(" [");
    Serial.print(arg6, BIN);
    Serial.print("], 0x");
    Serial.print(arg7, HEX);
    Serial.print(" [");
    Serial.print(arg7, BIN);
    Serial.println("])");
    Serial.flush();
#endif

    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        SPI.transfer(SI4735_CP_WRITE8);
        SPI.transfer(command);
        SPI.transfer(arg1);
        SPI.transfer(arg2);
        SPI.transfer(arg3);
        SPI.transfer(arg4);
        SPI.transfer(arg5);
        SPI.transfer(arg6);
        SPI.transfer(arg7);
        //Datahseet calls for 5ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        digitalWrite(_pinSEN, HIGH);
#endif
    } else {
#if !defined(SI4735_NOI2C)
        Wire.beginTransmission(_i2caddr);
        Wire.write(command);
        Wire.write(arg1);
        Wire.write(arg2);
        Wire.write(arg3);
        Wire.write(arg4);
        Wire.write(arg5);
        Wire.write(arg6);
        Wire.write(arg7);
        Wire.endTransmission();
#endif
    };
    
    //Each command takes a different time to decode inside the chip; readiness
    //for next command and, indeed, availability/validity of reponse data is
    //being signalled by CTS in status byte.
    //Furthermore, the datasheet specifically mandates waiting for CTS to come
    //back up before doing anything else, *including* attempting to read back
    //the response from the last command sent.
    //Therefore, we poll for CTS coming back up after we send the command.
    do {
        status = getStatus();
    } while(!(status & SI4735_STATUS_CTS));
}

void Si4735::setFrequency(word frequency){
    switch(_mode){
        case SI4735_MODE_FM:
            sendCommand(SI4735_CMD_FM_TUNE_FREQ, 0x00, highByte(frequency),
                        lowByte(frequency));
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_AM_TUNE_FREQ, 0x00, highByte(frequency), 
                        lowByte(frequency), 0x00,
                        ((_mode == SI4735_MODE_SW) ? 0x01 : 0x00));
            break;
    }
    completeTune();
}

byte Si4735::getRevision(char* FW, char* CMP, char* REV, word* patch){
    sendCommand(SI4735_CMD_GET_REV);
    getResponse(_response);    

    if(FW) {
        FW[0] = _response[2];
        FW[1] = _response[3];
        FW[2] = '\0';
    }
    if(CMP) {
        CMP[0] = _response[6];
        CMP[1] = _response[7];
        CMP[2] = '\0';
    }
    if(REV) *REV = _response[8];
    if(patch) *patch = word(_response[4], _response[5]);
    
    return _response[1];
}

word Si4735::getFrequency(bool* valid){
    word frequency;
    
    switch(_mode){
        case SI4735_MODE_FM:            
            sendCommand(SI4735_CMD_FM_TUNE_STATUS);
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_AM_TUNE_STATUS);
            break;
    }    
    getResponse(_response);
    frequency = word(_response[2], _response[3]);

    if(valid) *valid = (_response[1] & SI4735_STATUS_VALID);
    return frequency;
}

void Si4735::seekUp(bool wrap){
    switch(_mode){
        case SI4735_MODE_FM:
            sendCommand(SI4735_CMD_FM_SEEK_START, 
                        (SI4735_FLG_SEEKUP | 
                         (wrap ? SI4735_FLG_WRAP : 0x00)));
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_AM_SEEK_START, 
                        (SI4735_FLG_SEEKUP | (wrap ? SI4735_FLG_WRAP : 0x00)),
                        0x00, 0x00, 0x00, 
                        ((_mode == SI4735_MODE_SW) ? 0x01 : 0x00));
            break;
    }
    completeTune();
}

void Si4735::seekDown(bool wrap){
    switch(_mode){
        case SI4735_MODE_FM:
            sendCommand(SI4735_CMD_FM_SEEK_START, 
                        (wrap ? SI4735_FLG_WRAP : 0x00));
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_AM_SEEK_START, 
                        (wrap ? SI4735_FLG_WRAP : 0x00), 0x00, 0x00, 0x00, 
                        ((_mode == SI4735_MODE_SW) ? 0x01 : 0x00));
            break;
    }
    completeTune();
}

void Si4735::setSeekThresholds(byte SNR, byte RSSI){
    switch(_mode){
        case SI4735_MODE_FM:
            setProperty(SI4735_PROP_FM_SEEK_TUNE_SNR_THRESHOLD, 
                        word(0x00, constrain(SNR, 0, 127)));
            setProperty(SI4735_PROP_FM_SEEK_TUNE_RSSI_THRESHOLD,
                        word(0x00, constrain(RSSI, 0, 127)));                
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:    
            setProperty(SI4735_PROP_AM_SEEK_TUNE_SNR_THRESHOLD, 
                        word(0x00, constrain(SNR, 0, 63)));
            setProperty(SI4735_PROP_AM_SEEK_TUNE_RSSI_THRESHOLD,
                        word(0x00, constrain(RSSI, 0, 63)));                
            break;
    }
}

bool Si4735::readRDSBlock(word* block){
    //See if there's anything for us to do
    if(!(_mode == SI4735_MODE_FM && (getStatus() & SI4735_STATUS_RDSINT)))
        return false;
    
    _haverds = true;
    //Grab the next available RDS group from the chip
    sendCommand(SI4735_CMD_FM_RDS_STATUS, SI4735_FLG_INTACK);
    getResponse(_response);
    //memcpy() would be faster but it won't help since we're of a different
    //endianness than the device we're talking to.
    block[0] = word(_response[4], _response[5]);
    block[1] = word(_response[6], _response[7]);
    block[2] = word(_response[8], _response[9]);
    block[3] = word(_response[10], _response[11]);
    
    return true;
}

void Si4735::getRSQ(Si4735_RX_Metrics* RSQ){
    switch(_mode){
        case SI4735_MODE_FM:            
            sendCommand(SI4735_CMD_FM_RSQ_STATUS, SI4735_FLG_INTACK);            
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_AM_RSQ_STATUS, SI4735_FLG_INTACK);            
            break;
    }    
    //Now read the response    
    getResponse(_response);    

    //Pull the response data into their respecive fields
    RSQ->RSSI = _response[4];
    RSQ->SNR = _response[5];
    if(_mode == SI4735_MODE_FM){
        RSQ->PILOT = _response[3] & SI4735_STATUS_PILOT;
        RSQ->STBLEND = (_response[3] & (~SI4735_STATUS_PILOT));
        RSQ->MULT = _response[6];
        RSQ->FREQOFF = _response[7];
    }
}

bool Si4735::volumeUp(void){
    byte volume;

    volume = getVolume();
    if(volume < 63) {
        setVolume(++volume);
        return true;
    } else return false;
}

bool Si4735::volumeDown(bool alsomute){
    byte volume;

    volume = getVolume();
    if(volume > 0) {
        setVolume(--volume);
        return true;
    } else {
        if(alsomute) mute();
        return false;
    };
}

void Si4735::unMute(bool minvol){
    if(minvol) setVolume(0);
    setProperty(SI4735_PROP_RX_HARD_MUTE, word(0x00, 0x00));
}

byte Si4735::getStatus(void){
    byte response = 0;

    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        SPI.transfer(SI4735_CP_READ1_GPO1);
        response = SPI.transfer(0x00);
        //Datahseet calls for 5ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        digitalWrite(_pinSEN, HIGH);
#endif
    } else {
#if !defined(SI4735_NOI2C)
        Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)1);
        //I2C runs at 100kHz when using the Wire library, 100kHz = 10us period
        //so wait 10 bit-times for something to become available.
        while(!Wire.available()) delayMicroseconds(100);
        response = Wire.read();
#endif
    };
    return response;
}

void Si4735::getResponse(byte* response){
    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        SPI.transfer(SI4735_CP_READ16_GPO1);
        for(int i = 0; i < 16; i++) response[i] = SPI.transfer(0x00);
        //Datahseet calls for 5ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
        digitalWrite(_pinSEN, HIGH);
#endif
    } else {
#if !defined(SI4735_NOI2C)
        Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)16);
        for(int i = 0; i < 16; i++) {
            //I2C runs at 100kHz when using the Wire library, 100kHz = 10us
            //period so wait 10 bit-times for something to become available.
            while(!Wire.available()) delayMicroseconds(100);
            response[i] = Wire.read();          
        }
#endif
    };

#if defined(SI4735_DEBUG)
    Serial.print("Si4735 RSP");
    for(int i = 0; i < 4; i++) {
        if(i) Serial.print("           ");
        else Serial.print(" ");
        for(int j = 0; j < 4; j++) {
            Serial.print("0x");
            Serial.print(response[i * 4 + j], HEX);
            Serial.print(" [");
            Serial.print(response[i * 4 + j], BIN);
            Serial.print("]");
            if(j != 3) Serial.print(", ");
            else
                if(i != 3) Serial.print(",");
        }
        Serial.println("");
    }
    Serial.flush();
#endif
}

void Si4735::end(bool hardoff){
    sendCommand(SI4735_CMD_POWER_DOWN);
    if(hardoff) {
        //datasheet calls for 10ns, Arduino can only go as low as 3us
        delayMicroseconds(5);
#if !defined(SI4735_NOSPI)        
        if(!_i2caddr) SPI.end();
#endif
        digitalWrite(_pinReset, LOW);
        if(_pinPower != SI4735_PIN_POWER_HW) digitalWrite(_pinPower, LOW);
    };
}

void Si4735::setDeemphasis(byte deemph){
    switch(_mode){
        case SI4735_MODE_FM:            
            setProperty(SI4735_PROP_FM_DEEMPHASIS, word(0x00, deemph));        
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_LW:
        case SI4735_MODE_SW:
            setProperty(SI4735_PROP_AM_DEEMPHASIS, word(0x00, deemph));
            break;
    }
}

void Si4735::setMode(byte mode, bool powerdown, bool xosc){
    if(powerdown) end(false);
    _mode = mode;
    
    switch(_mode){
        case SI4735_MODE_FM:
            sendCommand(SI4735_CMD_POWER_UP, 
                        ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? 0x00 :
                         SI4735_FLG_GPO2IEN) | 
                        (xosc ? SI4735_FLG_XOSCEN : 0x00) | SI4735_FUNC_FM,
                        SI4735_OUT_ANALOG);
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(SI4735_CMD_POWER_UP,
                        ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? 0x00 :
                         SI4735_FLG_GPO2IEN) | 
                        (xosc ? SI4735_FLG_XOSCEN : 0x00) | SI4735_FUNC_AM,
                        SI4735_OUT_ANALOG);
            break;
    }

    //Configure GPO lines to maximize stability (see datasheet for discussion)
    //No need to do anything for GPO1 if using SPI
    //No need to do anything for GPO2 if using interrupts
    sendCommand(SI4735_CMD_GPIO_CTL, (_i2caddr ? SI4735_FLG_GPO1OEN : 0x00) |
                                     ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? 
                                     SI4735_FLG_GPO2OEN : 0x00));
    //Set GPO2 high if using interrupts as Si4735 has a LOW active INT line
    if(_pinGPO2 != SI4735_PIN_GPO2_HW)
      sendCommand(SI4735_CMD_GPIO_SET, SI4735_FLG_GPO2LEVEL);

    //Disable Mute
    unMute();

    //Set the seek band for the desired mode (AM and FM can use defaults)
    switch(_mode){
        case SI4735_MODE_SW:
            //Set the lower band limit for Short Wave Radio to 2.3 MHz
            setProperty(SI4735_PROP_AM_SEEK_BAND_BOTTOM, 0x08FC);            
            //Set the upper band limit for Short Wave Radio to 23 MHz
            setProperty(SI4735_PROP_AM_SEEK_BAND_TOP, 0x59D8);
            break;
        case SI4735_MODE_LW:
            //Set the lower band limit for Long Wave Radio to 152 kHz
            setProperty(SI4735_PROP_AM_SEEK_BAND_BOTTOM, 0x0099);            
            //Set the upper band limit for Long Wave Radio to 279 kHz
            setProperty(SI4735_PROP_AM_SEEK_BAND_BOTTOM, 0x0117);                            
            break;
    }
    
    //Enable end-of-seek and RDS interrupts, if we're actually using interrupts
    //TODO: write interrupt handlers for STCINT and RDSINT
    if(_pinGPO2 != SI4735_PIN_GPO2_HW)
      setProperty(
          SI4735_PROP_GPO_IEN, 
          word(0x00, ((_mode == SI4735_MODE_FM) ? SI4735_FLG_RDSIEN : 0x00) | 
                     SI4735_FLG_STCIEN));
}

void Si4735::setProperty(word property, word value){
    sendCommand(SI4735_CMD_SET_PROPERTY, 0x00, highByte(property), 
                lowByte(property), highByte(value), lowByte(value));
    //Datasheet states SET_PROPERTY completes 10ms after sending the command
    //irrespective of CTS coming up earlier than that
    delay(10);
}

word Si4735::getProperty(word property){    
    sendCommand(SI4735_CMD_GET_PROPERTY, 0x00, highByte(property), 
                lowByte(property));
    getResponse(_response);

    return word(_response[2], _response[3]);
}

void Si4735::enableRDS(void){
    //Enable and configure RDS reception
    if(_mode == SI4735_MODE_FM) {
        setProperty(SI4735_PROP_FM_RDS_INT_SOURCE, word(0x00, 
                                                        SI4735_FLG_RDSRECV));
        setProperty(SI4735_PROP_FM_RDS_INT_FIFO_COUNT, word(0x00, 0x01));
        setProperty(SI4735_PROP_FM_RDS_CONFIG, word(SI4735_FLG_BLETHA_35 | 
                    SI4735_FLG_BLETHB_35 | SI4735_FLG_BLETHC_35 | 
                    SI4735_FLG_BLETHD_35, SI4735_FLG_RDSEN));
    };
}

void Si4735::waitForInterrupt(byte which){
    while(!(getStatus() & which)){
        //Balance being snappy with hogging the chip
        delay(125);
        sendCommand(SI4735_CMD_GET_INT_STATUS);
    }
}

void Si4735::completeTune(void) {
    waitForInterrupt(SI4735_STATUS_STCINT);
    //Make future off-to-on STCINT transitions visible
    switch(_mode){
        case SI4735_MODE_FM:
                sendCommand(SI4735_FM_TUNE_STATUS, SI4735_FLG_INTACK);
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
                sendCommand(SI4735_AM_TUNE_STATUS, SI4735_FLG_INTACK);
            break;
    }
    if(_mode == SI4735_MODE_FM) enableRDS();
}