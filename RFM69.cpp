// RFM69.cpp
//
// Copyright (C) 2014 Phil Crump
//
// Based on RF22 Copyright (C) 2011 Mike McCauley ported to mbed by Karl Zweimueller
// Based on RFM69 LowPowerLabs (https://github.com/LowPowerLab/RFM69/)


#include "mbed.h"
#include "RFM69.h"


RFM69::RFM69(PinName slaveSelectPin, PinName mosi, PinName miso, PinName sclk, PinName interrupt)
    : _slaveSelectPin(slaveSelectPin),  _spi(mosi, miso, sclk), _interrupt(interrupt) /*, led1(LED1), led2(LED2), led3(LED3), led4(LED4) */
{
    _idleMode = RF69_MODE_RCON; // Default idle state is IDLE, with RC osc running, typ: 1.2uA
    _mode = RF69_MODE_RX; // We start up in idle mode
    _rxGood = 0;
    _rxBad = 0;
    _txGood = 0;
    _afterTxMode = RF69_MODE_RX;
}

boolean RFM69::init()
{
    wait_ms(16);

    _slaveSelectPin = 1; // Init nSS

    wait_ms(100);

    // Setup the spi for 8 bit data : 1RW-bit 7 adressbit and  8 databit
    // second edge capture, with a 1MHz clock rate
    _spi.format(8,0);
    _spi.frequency(1000000);

    // Software reset the device
    reset();

    // Get the device type and check it
    // This also tests whether we are really connected to a device
    _deviceType = spiRead(RF22_REG_00_DEVICE_TYPE);
    if (   _deviceType != RF22_DEVICE_TYPE_RX_TRX
            && _deviceType != RF22_DEVICE_TYPE_TX)
        return false;

    // Set up Interrupt
    _interrupt.fall(this, &RFM69::isr0);

    clearTxBuf();
    clearRxBuf();


    // Enable interrupts
    0x25 RegDioMapping1 
    0x26 RegDioMapping2
    spiWrite(RFM69_REG_25_DIO_MAPPING1, 0x00); // Set DIO Mapping to default

    spiWrite(RFM69_REG_26_DIO_MAPPING2, RFM69_CLKOUT_OFF); // Switch off Clkout


    // Set some defaults. An innocuous ISM frequency, and reasonable pull-in
    setFrequency(869.5, 0.05);


    setModemConfig(FSK_Rb2_4Fd36);
    // Minimum power
    setTxPower(RF22_TXPOW_8DBM);
//    setTxPower(RF22_TXPOW_17DBM);



    return true;
}

void RFM69::handleInterrupt()
{
    uint8_t _lastInterruptFlags[2];

    // RX
    if(_mode == RFM69_MODE_RX) {
    
        // CRCOK (incoming packet)
        if(readReg(RFM69_REG_28_IRQFLAGS2) & RFM69_IRQFLAGS2_CRCOK) {
        
    // TX
    } else if(_mode == RFM69_MODE_TX) {
    
        // PacketSent
        if(readReg(RFM69_REG_28_IRQFLAGS2) & RFM69_IRQFLAGS2_PACKETSENT) {
            _txGood++;
            setMode(_afterTxMode); // IMPLEMENT THIS!!
        }
    }
   
    if (_lastInterruptFlags[0] & RF22_IPKVALID) {
        uint8_t len = readReg(RFM69_REG_38_PAYLOADLENGTH);

        // May have already read one or more fragments
        // Get any remaining unread octets, based on the expected length
        // First make sure we dont overflow the buffer in the case of a stupid length
        // or partial bad receives
        /*
        if (len >  RF69_MAX_MESSAGE_LEN || len < _bufLen) {
            _rxBad++;
            //led2 = !led2;
            _mode = RF22_MODE_IDLE;
            clearRxBuf();
            return; // Hmmm receiver buffer overflow.
        }
        */
        spiBurstRead(RFM69_REG_00_FIFO_ACCESS, _buf + _bufLen, len - _bufLen);
        _rxGood++;
        _bufLen = len;
        _mode = RF22_MODE_IDLE;
        _rxBufValid = true;
    }
}

void RFM69::isr0()
{
    handleInterrupt();
}

uint8_t RFM69::spiRead(uint8_t reg)
{
    __disable_irq();    // Disable Interrupts
    _slaveSelectPin=0;
    
    _spi.write(reg & ~RFM69_SPI_WRITE_MASK); // Send the address with the write mask off
    uint8_t val = _spi.write(0); // The written value is ignored, reg value is read
    
    _slaveSelectPin = 1;
    __enable_irq();     // Enable Interrupts
    return val;
}

void RFM69::spiWrite(uint8_t reg, uint8_t val)
{
    __disable_irq();    // Disable Interrupts
    _slaveSelectPin = 0;
    
    _spi.write(reg | RFM69_SPI_WRITE_MASK); // Send the address with the write mask on
    _spi.write(val); // New value follows

    _slaveSelectPin = 1;
    __enable_irq();     // Enable Interrupts
}

void RFM69::spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len)
{
    _slaveSelectPin = 0;
    
    _spi.write(reg & ~RFM69_SPI_WRITE_MASK); // Send the start address with the write mask off
    while (len--)
        *dest++ = _spi.write(0);

    _slaveSelectPin = 1;
}

void RFM69::spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
    _slaveSelectPin = 0;
    
    _spi.write(reg | RFM69_SPI_WRITE_MASK); // Send the start address with the write mask on
    while (len--)
        _spi.write(*src++);
        
    _slaveSelectPin = 1;
}
/*
uint8_t RFM69::adcRead(uint8_t adcsel,
                      uint8_t adcref ,
                      uint8_t adcgain,
                      uint8_t adcoffs)
{
    uint8_t configuration = adcsel | adcref | (adcgain & RF22_ADCGAIN);
    spiWrite(RF22_REG_0F_ADC_CONFIGURATION, configuration | RF22_ADCSTART);
    spiWrite(RF22_REG_10_ADC_SENSOR_AMP_OFFSET, adcoffs);

    // Conversion time is nominally 305usec
    // Wait for the DONE bit
    while (!(spiRead(RF22_REG_0F_ADC_CONFIGURATION) & RF22_ADCDONE))
        ;
    // Return the value
    return spiRead(RF22_REG_11_ADC_VALUE);
}

uint8_t RFM69::temperatureRead(uint8_t tsrange, uint8_t tvoffs)
{
    spiWrite(RF22_REG_12_TEMPERATURE_SENSOR_CALIBRATION, tsrange | RF22_ENTSOFFS);
    spiWrite(RF22_REG_13_TEMPERATURE_VALUE_OFFSET, tvoffs);
    return adcRead(RF22_ADCSEL_INTERNAL_TEMPERATURE_SENSOR | RF22_ADCREF_BANDGAP_VOLTAGE);
}

uint16_t RFM69::wutRead()
{
    uint8_t buf[2];
    spiBurstRead(RF22_REG_17_WAKEUP_TIMER_VALUE1, buf, 2);
    return ((uint16_t)buf[0] << 8) | buf[1]; // Dont rely on byte order
}

// RFM-22 doc appears to be wrong: WUT for wtm = 10000, r, = 0, d = 0 is about 1 sec
void RF22::setWutPeriod(uint16_t wtm, uint8_t wtr, uint8_t wtd)
{
    uint8_t period[3];

    period[0] = ((wtr & 0xf) << 2) | (wtd & 0x3);
    period[1] = wtm >> 8;
    period[2] = wtm & 0xff;
    spiBurstWrite(RF22_REG_14_WAKEUP_TIMER_PERIOD1, period, sizeof(period));
}
*/

uint8_t RFM69::rssiRead() // Converted
{
    int rssi = 0;
    if (forceTrigger) {
        //RSSI trigger not needed if DAGC is in continuous mode
        spiWrite(RFM69_REG_23_RSSI_CONFIG, RFM69_RSSI_START);
        while ((spiRead(RFM69_REG_23_RSSI_CONFIG) & RFM69_RSSI_DONE) == 0x00); // Wait for RSSI_Ready
    }
    rssi = -spiRead(RFM69_REG_24_RSSI_VALUE);
    rssi >>= 1;
    return rssi;
}

void RFM69::setMode(uint8_t newMode) // Converted
{
    spiWrite(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | newMode);

	_mode = newMode;
}
uint8_t  RFM69::mode()
{
    return _mode;
}

void RFM69::setTxPower(uint8_t power)
{
    spiWrite(RF22_REG_6D_TX_POWER, power);
}

// Set one of the canned FSK Modem configs
// Returns true if its a valid choice
boolean RF22::setModemConfig(ModemConfigChoice index)
{
    if (index > (sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
        return false;

    RF22::ModemConfig cfg;
    memcpy(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(RF22::ModemConfig));
    setModemRegisters(&cfg);

    return true;
}

void RF22::clearRxBuf()
{
    __disable_irq();    // Disable Interrupts
    _bufLen = 0;
    _rxBufValid = false;
    __enable_irq();     // Enable Interrupts
}

boolean RF22::available()
{
    if (!_rxBufValid)
        setMode(RFM69_MODE_RX); // Make sure we are receiving - Do we need this?
    return _rxBufValid;
}

boolean RF22::recv(uint8_t* buf, uint8_t* len)
{
    if (!available())
        return false;
    __disable_irq();    // Disable Interrupts
    if (*len > _bufLen)
        *len = _bufLen;
    memcpy(buf, _buf, *len);
    clearRxBuf();
    __enable_irq();     // Enable Interrupts
//    printBuffer("recv:", buf, *len);
//    }
    return true;
}

void RF22::clearTxBuf()
{
    __disable_irq();    // Disable Interrupts
    _bufLen = 0;
    _txBufSentIndex = 0;
    _txPacketSent = false;
    __enable_irq();     // Enable Interrupts
}

void RF22::startTransmit()
{
    sendNextFragment(); // Actually the first fragment
    spiWrite(RF22_REG_3E_PACKET_LENGTH, _bufLen); // Total length that will be sent
    setModeTx(); // Start the transmitter, turns off the receiver
}

// Restart the transmission of a packet that had a problem
void RF22::restartTransmit()
{
    _mode = RF22_MODE_IDLE;
    _txBufSentIndex = 0;
//        Serial.println("Restart");
    startTransmit();
}

boolean RF22::send(const uint8_t* data, uint8_t len)
{
    waitPacketSent();
//    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (!fillTxBuf(data, len))
            return false;
        startTransmit();
    }
//    printBuffer("send:", data, len);
    return true;
}

boolean RF22::fillTxBuf(const uint8_t* data, uint8_t len)
{
    //clearTxBuf();
    if (!len)
        return false;
    return appendTxBuf(data, len);
}

boolean RF22::appendTxBuf(const uint8_t* data, uint8_t len)
{
    if (((uint16_t)_bufLen + len) > RFM69_MAX_MESSAGE_LEN)
        return false;
    __disable_irq();    // Disable Interrupts
    memcpy(_buf + _bufLen, data, len);
    _bufLen += len;
    __enable_irq();     // Enable Interrupts

//    printBuffer("txbuf:", _buf, _bufLen);
    return true;
}

// Assumption: there is currently <= RF22_TXFFAEM_THRESHOLD bytes in the Tx FIFO
void RF22::sendNextFragment()
{
    if (_txBufSentIndex < _bufLen) {
        // Some left to send?
        uint8_t len = _bufLen - _txBufSentIndex;
        // But dont send too much
        if (len > (RF22_FIFO_SIZE - RF22_TXFFAEM_THRESHOLD - 1))
            len = (RF22_FIFO_SIZE - RF22_TXFFAEM_THRESHOLD - 1);
        spiBurstWrite(RF22_REG_7F_FIFO_ACCESS, _buf + _txBufSentIndex, len);
        _txBufSentIndex += len;
    }
}

// Assumption: there are at least RF22_RXFFAFULL_THRESHOLD in the RX FIFO
// That means it should only be called after a RXFFAFULL interrupt
void RF22::readNextFragment()
{
    if (((uint16_t)_bufLen + RF22_RXFFAFULL_THRESHOLD) > RF22_MAX_MESSAGE_LEN)
        return; // Hmmm receiver overflow. Should never occur

    // Read the RF22_RXFFAFULL_THRESHOLD octets that should be there
    spiBurstRead(RF22_REG_7F_FIFO_ACCESS, _buf + _bufLen, RF22_RXFFAFULL_THRESHOLD);
    _bufLen += RF22_RXFFAFULL_THRESHOLD;
}

uint8_t RF22::lastRssi()
{
    return _lastRssi;
}
