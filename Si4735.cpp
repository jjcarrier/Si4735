/* Arduino Si4735 (and family) Library
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

void Si4735::begin(byte mode, bool xosc, bool slowshifter, bool interrupt){
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
    //but Arduino can only go as low as ~1us.
    delayMicroseconds(1);
    digitalWrite(_pinReset, HIGH);
    //Datasheet calls for 30ns delay; an Arduino running at 20MHz (4MHz
    //faster than the Uno. mind you) has a clock period of 50ns so no action
    //needed.

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

    setMode(mode, false, xosc, interrupt);
}

void Si4735::sendCommand(byte command, byte arg1, byte arg2, byte arg3,
                         byte arg4, byte arg5, byte arg6, byte arg7){
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
    if (_seeking) {
      //The datasheet strongly recommends that no other command (not only a tune
      //or seek one and except GET_INT_STATUS) is sent until the current
      //seek/tune operation is complete.
      //NOTE: the datasheet makes it clear STC implies CTS.
      waitForInterrupt(SI4735_STATUS_STCINT);
      _seeking = false;
    } else waitForInterrupt(SI4735_STATUS_CTS);
    sendCommandInternal(command, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}

void Si4735::sendCommandInternal(byte command, byte arg1, byte arg2, byte arg3,
                                 byte arg4, byte arg5, byte arg6, byte arg7){
    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns delay; an Arduino running at 20MHz (4MHz
        //faster than the Uno. mind you) has a clock period of 50ns so no action
        //needed.
        SPI.transfer(SI4735_CP_WRITE8);
        SPI.transfer(command);
        SPI.transfer(arg1);
        SPI.transfer(arg2);
        SPI.transfer(arg3);
        SPI.transfer(arg4);
        SPI.transfer(arg5);
        SPI.transfer(arg6);
        SPI.transfer(arg7);
        //Datasheet calls for 5ns delay; an Arduino running at 20MHz (4MHz
        //faster than the Uno. mind you) has a clock period of 50ns so no action
        //needed.
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
};

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

bool Si4735::readRDSGroup(word* block){
    //See if there's anything for us to do
    if(!(getStatus() & SI4735_STATUS_RDSINT))
        return false;

    //Grab the next available RDS group from the chip
    sendCommand(SI4735_CMD_FM_RDS_STATUS, SI4735_FLG_INTACK);
    getResponse(_response);
    //Of course, we got here because the chip just interrupted us to tell it has
    //received RDS data -- so much of it that the FIFO high-watermark was hit.
    //Still, it never hurts to be consistent so we'll set _haverds to the chip's
    //version of the facts (as opposed to a hardcoded true).
    _haverds = _response[1] & SI4735_FLG_RDSSYNCFOUND;
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

void Si4735::updateStatus(void){
    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns delay; an Arduino running at 20MHz (4MHz
        //faster than the Uno. mind you) has a clock period of 50ns so no action
        //needed.
        SPI.transfer(SI4735_CP_READ1_GPO1);
        _status = SPI.transfer(0x00);
        //Datahseet calls for 5ns delay; see comment above.
        digitalWrite(_pinSEN, HIGH);
#endif
    } else {
#if !defined(SI4735_NOI2C)
        Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)1);
        //I2C runs at 100kHz when using the Wire library, 100kHz = 10us period
        //so wait 10 bit-times for something to become available.
        while(!Wire.available()) delayMicroseconds(100);
        _status = Wire.read();
#endif
    };
};

byte Si4735::getStatus(void){
    if(!_interrupt)
        updateStatus();

    return _status;
}

void Si4735::getResponse(byte* response){
    if(!_i2caddr) {
#if !defined(SI4735_NOSPI)
        digitalWrite(_pinSEN, LOW);
        //Datasheet calls for 30ns delay; an Arduino running at 20MHz (4MHz
        //faster than the Uno. mind you) has a clock period of 50ns so no action
        //needed.
        SPI.transfer(SI4735_CP_READ16_GPO1);
        for(int i = 0; i < 16; i++) response[i] = SPI.transfer(0x00);
        //Datahseet calls for 5ns delay; see above comment.
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
        //Datasheet calls for 10ns delay; an Arduino running at 20MHz (4MHz
        //faster than the Uno. mind you) has a clock period of 50ns so no action
        //needed.
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

void Si4735::setMode(byte mode, bool powerdown, bool xosc, bool interrupt){
    if(powerdown) end(false);
    _mode = mode;
    _seeking = false;
    //Everything below is done in polling mode as interrupt setup is incomplete.
    if (_interrupt)
      detachInterrupt(_pinGPO2);
    _interrupt = false;

    switch(_mode){
        case SI4735_MODE_FM:
            sendCommand(
                SI4735_CMD_POWER_UP,
                SI4735_FLG_CTSIEN |
                ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? 0x00 : SI4735_FLG_GPO2IEN) |
                (xosc ? SI4735_FLG_XOSCEN : 0x00) | SI4735_FUNC_FM,
                SI4735_OUT_ANALOG);
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
            sendCommand(
                SI4735_CMD_POWER_UP,
                SI4735_FLG_CTSIEN |
                ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? 0x00 : SI4735_FLG_GPO2IEN) |
                (xosc ? SI4735_FLG_XOSCEN : 0x00) | SI4735_FUNC_AM,
                SI4735_OUT_ANALOG);
            break;
    }

    //Configure GPO lines to maximize stability (see datasheet for discussion)
    //No need to do anything for GPO1 if using SPI
    //No need to do anything for GPO2 if using interrupts
    sendCommand(SI4735_CMD_GPIO_CTL,
                (_i2caddr ? SI4735_FLG_GPO1OEN : 0x00) |
                ((_pinGPO2 == SI4735_PIN_GPO2_HW) ? SI4735_FLG_GPO2OEN : 0x00));
    //Set GPO2 high if using interrupts as Si4735 has a LOW active INT line
    if(_pinGPO2 != SI4735_PIN_GPO2_HW)
      sendCommand(SI4735_CMD_GPIO_SET, SI4735_FLG_GPO2LEVEL);

    //Enable CTS, end-of-seek and RDS interrupts (if in FM mode)
    if(_pinGPO2 != SI4735_PIN_GPO2_HW)
      setProperty(
          SI4735_PROP_GPO_IEN,
          word(0x00, (
              SI4735_FLG_CTSIEN |
              (_mode == SI4735_MODE_FM) ? SI4735_FLG_RDSIEN : 0x00) |
              SI4735_FLG_STCIEN));

    //The chip is alive and interrupts have been configured on its side, switch
    //ourselves to interrupt operation if so requested and if wiring was
    //properly done.
    _interrupt = interrupt && _pinGPO2 != SI4735_PIN_GPO2_HW;

    if (_interrupt) {
      attachInterrupt(_pinGPO2, Si4735::interruptServiceRoutine, FALLING);
      interrupts();
    };

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
}

void Si4735::setProperty(word property, word value){
    sendCommand(SI4735_CMD_SET_PROPERTY, 0x00, highByte(property),
                lowByte(property), highByte(value), lowByte(value));
    //Datasheet states SET_PROPERTY completes 10ms after sending the command
    //irrespective of CTS coming up earlier than that, so we wait anyway.
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
        //Set the FIFO high-watermark to 12 RDS blocks, which is safe even for
        //old chips, yet large enough to improve performance.
        setProperty(SI4735_PROP_FM_RDS_INT_FIFO_COUNT, word(0x00, 0x0C));
        setProperty(SI4735_PROP_FM_RDS_CONFIG, word(SI4735_FLG_BLETHA_35 |
                    SI4735_FLG_BLETHB_35 | SI4735_FLG_BLETHC_35 |
                    SI4735_FLG_BLETHD_35, SI4735_FLG_RDSEN));
    };
}

void Si4735::waitForInterrupt(byte which){
    while(!(getStatus() & which))
      if(!_interrupt)
        if (which == SI4735_STATUS_STCINT)
          //According to the datasheet, the chip would prefer we don't disturb
          //it with serial communication while it's seeking or tuning into a
          //station. Sleep for two channel seek-times to give it a rest.
          //NOTE: this means seek/tune operations will not complete in less than
          //120ms, regardless of signal quality. If you don't like this, switch
          //to interrupt mode (like you should have, from the beginning).
          delay(120);
        sendCommand(SI4735_CMD_GET_INT_STATUS);
}

void Si4735::completeTune(void) {
    //Make sendCommand() below block until the seek/tune operation completes.
    _seeking = true;
    //Make future off-to-on STCINT transitions visible (again).
    switch(_mode){
        case SI4735_MODE_FM:
                sendCommand(SI4735_CMD_FM_TUNE_STATUS, SI4735_FLG_INTACK);
            break;
        case SI4735_MODE_AM:
        case SI4735_MODE_SW:
        case SI4735_MODE_LW:
                sendCommand(SI4735_CMD_AM_TUNE_STATUS, SI4735_FLG_INTACK);
            break;
    }
    if(_mode == SI4735_MODE_FM) enableRDS();
}

void Si4735::interruptServiceRoutine(void) {
    static volatile bool _getIntStatus = false;

    if (!_getIntStatus) {
      //Datasheet is clear on the fact that CTS will be asserted before any
      //command completes (i.e. decoding always takes less than execution);
      //therefore we can always send GET_INT_STATUS here since we were just
      //interrupted by the chip telling us it's at least ready for the next
      //command.
      sendCommandInternal(SI4735_CMD_GET_INT_STATUS);
      _getIntStatus = true;
    } else {
      //The *INT bits in the status byte are now guaranteed to be updated.
      updateStatus();
      //Re-arm flip-flop.
      _getIntStatus = false;
    };
};

volatile byte Si4735::_status = 0x00;
byte Si4735::_pinSEN = SI4735_PIN_SEN;
byte Si4735::_i2caddr = 0x00;
