// RFM69.cpp
//
// Copyright (C) 2014 Phil Crump
//
// Based on RF22 Copyright (C) 2011 Mike McCauley ported to mbed by Karl Zweimueller
// Based on RFM69 LowPowerLabs (https://github.com/LowPowerLab/RFM69/)


#include "mbed.h"
#include "RFM69.h"
#include "RFM69Config.h"


RFM69::RFM69(PinName slaveSelectPin, PinName mosi, PinName miso, PinName sclk, PinName interrupt)
    : _slaveSelectPin(slaveSelectPin),  _spi(mosi, miso, sclk), _interrupt(interrupt) /*, led1(LED1), led2(LED2), led3(LED3), led4(LED4) */
{
    _idleMode = RFM69_MODE_SLEEP; // Default idle state is SLEEP, our lowest power mode
    _mode = RFM69_MODE_RX; // We start up in RX mode
    _rxGood = 0;
    _rxBad = 0;
    _txGood = 0;
    _afterTxMode = RFM69_MODE_RX;
}

boolean RFM69::init()
{
    wait_ms(12);

    _slaveSelectPin = 1; // Init nSS

    wait_ms(100);

    _spi.format(8,0);
    _spi.frequency(1000000); // 1MHz (increase to 10MHz after testing)

    // We should check for a device here, maybe set and check mode?
    
    _interrupt.fall(this, &RFM69::isr0);

    clearTxBuf();
    clearRxBuf();


    // Set up GPIOs
    spiWrite(RFM69_REG_25_DIO_MAPPING1, 0x00); // Set DIO Mapping to default
    spiWrite(RFM69_REG_26_DIO_MAPPING2, RFM69_CLKOUT_OFF); // Switch off Clkout

    setFrequency(869.5, 0.05);

//    setModemConfig(FSK_Rb2_4Fd36);
    // Minimum power
//    setTxPower(RF22_TXPOW_8DBM);
//    setTxPower(RF22_TXPOW_17DBM);



    return true;
}

void RFM69::handleInterrupt()
{
    // RX
    if(_mode == RFM69_MODE_RX) {
    
        // CRCOK (incoming packet)
        if(spiRead(RFM69_REG_28_IRQ_FLAGS2) & RF_IRQFLAGS2_CRCOK) {
            spiBurstRead(RFM69_REG_00_FIFO, _buf + _bufLen, len - _bufLen);
            _rxGood++;
            _bufLen = len;
            _rxBufValid = true;
        }
    // TX
    } else if(_mode == RFM69_MODE_TX) {
    
        // PacketSent
        if(spiRead(RFM69_REG_28_IRQ_FLAGS2) & RF_IRQFLAGS2_PACKETSENT) {
            _txGood++;
            setMode(_afterTxMode);
            _txPacketSent = true;
        }
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

uint8_t RFM69::rssiRead()
{
    int rssi = 0;
    //RSSI trigger not needed if DAGC is in continuous mode
    spiWrite(RFM69_REG_23_RSSI_CONFIG, RF_RSSI_START);
    while ((spiRead(RFM69_REG_23_RSSI_CONFIG) & RF_RSSI_DONE) == 0x00); // Wait for RSSI_Ready, make this asynchronous sometime?
    rssi = -spiRead(RFM69_REG_24_RSSI_VALUE);
    rssi >>= 1;
    return rssi;
}

void RFM69::setMode(uint8_t newMode)
{
    spiWrite(RFM69_REG_01_OPMODE, (spiRead(RFM69_REG_01_OPMODE) & 0xE3) | newMode);
	_mode = newMode;
}
void RFM69::setModeSleep()
{
    setMode(RFM69_MODE_SLEEP);
}
void RFM69::setModeRx()
{
    setMode(RFM69_MODE_RX);
}
void RFM69::setModeTx()
{
    setMode(RFM69_MODE_TX);
}
uint8_t  RFM69::mode()
{
    return _mode;
}

void RFM69::clearRxBuf()
{
    __disable_irq();    // Disable Interrupts
    _bufLen = 0;
    _rxBufValid = false;
    __enable_irq();     // Enable Interrupts
}

boolean RFM69::available()
{
    if (!_rxBufValid)
        setMode(RFM69_MODE_RX); // Make sure we are receiving - Do we need this?
    return _rxBufValid;
}

boolean RFM69::recv(uint8_t* buf, uint8_t* len)
{
    if (!available())
        return false;
    __disable_irq();    // Disable Interrupts
    if (*len > _bufLen)
        *len = _bufLen;
    memcpy(buf, _buf, *len);
    clearRxBuf();
    __enable_irq();     // Enable Interrupts
    return true;
}

void RFM69::clearTxBuf()
{
    __disable_irq();    // Disable Interrupts
    _bufLen = 0;
    _txBufSentIndex = 0;
    _txPacketSent = false;
    __enable_irq();     // Enable Interrupts
}

void RFM69::startTransmit()
{
    //sendNextFragment(); // Actually the first fragment
    setModeTx(); // Start the transmitter, turns off the receiver
    sendTxBuf();
}

boolean RFM69::send(const uint8_t* data, uint8_t len)
{
//    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (!fillTxBuf(data, len))
            return false;
        startTransmit();
    }
    return true;
}

boolean RFM69::fillTxBuf(const uint8_t* data, uint8_t len)
{
    if (((uint16_t)_bufLen + len) > RFM69_MAX_MESSAGE_LEN)
        return false;
    __disable_irq();    // Disable Interrupts
    memcpy(_buf + _bufLen, data, len);
    _bufLen += len;
    __enable_irq();     // Enable Interrupts
    return true;
}

void RFM69::sendTxBuf() {
    if(_bufLen<RFM69_FIFO_SIZE) {
        uint8_t len = _bufLen;
        spiWrite(RFM69_REG_00_FIFO, len); // Send length byte
        spiBurstWrite(RFM69_REG_00_FIFO, _buf, len);
    }
}

void RFM69::readRxBuf()
{
    spiBurstRead(RFM69_REG_00_FIFO, _buf, RFM69_FIFO_SIZE);
    _bufLen += RFM69_FIFO_SIZE;
}

uint8_t RFM69::lastRssi()
{
    return _lastRssi;
}
