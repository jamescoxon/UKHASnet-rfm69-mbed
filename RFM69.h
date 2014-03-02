// RFM69.h
// Author: Phil Crump (phildcrump@gmail.com)
// Copyright (C) 2014 Phil Crump
// Based on RF22.h
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2011 Mike McCauley

#ifndef RFM69_h
#define RFM69_h
#include "mbed.h"

#define boolean bool

#define RFM69_SPI_WRITE_MASK 0x80

// This is the maximum message length that can be supported by this library. Limited by
// the single message length octet in the header. 
// Yes, 255 is correct even though the FIFO size in the RF22 is only
// 64 octets. We use interrupts to refill the Tx FIFO during transmission and to empty the
// Rx FIFO during reception
// Can be pre-defined to a smaller size (to save SRAM) prior to including this header
#define RFM69_MAX_MESSAGE_LEN 65

// Max number of octets the RF22 Rx and Tx FIFOs can hold
#define RFM69_FIFO_SIZE 64

#define RFM69_MODE_SLEEP    0x00 0.1uA
#define RFM69_MODE_STDBY    0x04 1.25mA
#define RFM69_MODE_RX       0x10 16mA
#define RFM69_MODE_TX       0x0c >=33mA


// These values we set for FIFO thresholds are actually the same as the POR values
#define RF22_TXFFAEM_THRESHOLD 4
#define RF22_RXFFAFULL_THRESHOLD 55


// Number of registers to be passed to setModemConfig()
#define RF22_NUM_MODEM_CONFIG_REGS 18

// Register names


#define RFM69_REG_00_FIFO           0x00
#define RFM69_REG_01_OPMODE         0x01
#define RFM69_REG_23_RSSI_CONFIG    0x23
#define RFM69_REG_24_RSSI_VALUE     0x24
#define RFM69_REG_25_DIO_MAPPING1   0x25
#define RFM69_REG_26_DIO_MAPPING2   0x26


class RF22
{
public:

    /// \brief Defines register values for a set of modem configuration registers
    ///
    /// Defines register values for a set of modem configuration registers
    /// that can be passed to setModemConfig()
    /// if none of the choices in ModemConfigChoice suit your need
    /// setModemConfig() writes the register values to the appropriate RF22 registers
    /// to set the desired modulation type, data rate and deviation/bandwidth.
    /// Suitable values for these registers can be computed using the register calculator at
    /// http://www.hoperf.com/upload/rf/RF22B%2023B%2031B%2042B%2043B%20Register%20Settings_RevB1-v5.xls
    typedef struct
    {
    uint8_t    reg_1c;   ///< Value for register RF22_REG_1C_IF_FILTER_BANDWIDTH
    uint8_t    reg_1f;   ///< Value for register RF22_REG_1F_CLOCK_RECOVERY_GEARSHIFT_OVERRIDE
    uint8_t    reg_20;   ///< Value for register RF22_REG_20_CLOCK_RECOVERY_OVERSAMPLING_RATE
    uint8_t    reg_21;   ///< Value for register RF22_REG_21_CLOCK_RECOVERY_OFFSET2 
    uint8_t    reg_22;   ///< Value for register RF22_REG_22_CLOCK_RECOVERY_OFFSET1 
    uint8_t    reg_23;   ///< Value for register RF22_REG_23_CLOCK_RECOVERY_OFFSET0
    uint8_t    reg_24;   ///< Value for register RF22_REG_24_CLOCK_RECOVERY_TIMING_LOOP_GAIN1
    uint8_t    reg_25;   ///< Value for register RF22_REG_25_CLOCK_RECOVERY_TIMING_LOOP_GAIN0 
    uint8_t    reg_2c;   ///< Value for register RF22_REG_2C_OOK_COUNTER_VALUE_1 
    uint8_t    reg_2d;   ///< Value for register RF22_REG_2D_OOK_COUNTER_VALUE_2
    uint8_t    reg_2e;   ///< Value for register RF22_REG_2E_SLICER_PEAK_HOLD 
    uint8_t    reg_58;   ///< Value for register RF22_REG_58_CHARGE_PUMP_CURRENT_TRIMMING
    uint8_t    reg_69;   ///< Value for register RF22_REG_69_AGC_OVERRIDE1 
    uint8_t    reg_6e;   ///< Value for register RF22_REG_6E_TX_DATA_RATE1
    uint8_t    reg_6f;   ///< Value for register RF22_REG_6F_TX_DATA_RATE0 
    uint8_t    reg_70;   ///< Value for register RF22_REG_70_MODULATION_CONTROL1
    uint8_t    reg_71;   ///< Value for register RF22_REG_71_MODULATION_CONTROL2
    uint8_t    reg_72;   ///< Value for register RF22_REG_72_FREQUENCY_DEVIATION
    } ModemConfig;
  
    /// Choices for setModemConfig() for a selected subset of common modulation types,
    /// and data rates. If you need another configuration, use the register calculator.
    /// and call setModemRegisters() with your desired settings
    /// These are indexes into _modemConfig
    typedef enum
    {
    UnmodulatedCarrier = 0, ///< Unmodulated carrier for testing
    FSK_PN9_Rb2Fd5,      ///< FSK, No Manchester, Rb = 2kbs, Fd = 5kHz, PN9 random modulation for testing

    FSK_Rb2Fd5,         ///< FSK, No Manchester, Rb = 2kbs,    Fd = 5kHz
    FSK_Rb2_4Fd36,       ///< FSK, No Manchester, Rb = 2.4kbs,  Fd = 36kHz
    FSK_Rb4_8Fd45,       ///< FSK, No Manchester, Rb = 4.8kbs,  Fd = 45kHz
    FSK_Rb9_6Fd45,       ///< FSK, No Manchester, Rb = 9.6kbs,  Fd = 45kHz
    FSK_Rb19_2Fd9_6,     ///< FSK, No Manchester, Rb = 19.2kbs, Fd = 9.6kHz
    FSK_Rb38_4Fd19_6,    ///< FSK, No Manchester, Rb = 38.4kbs, Fd = 19.6kHz
    FSK_Rb57_6Fd28_8,    ///< FSK, No Manchester, Rb = 57.6kbs, Fd = 28.8kHz
    FSK_Rb125Fd125,      ///< FSK, No Manchester, Rb = 125kbs,  Fd = 125kHz

    GFSK_Rb2Fd5,         ///< GFSK, No Manchester, Rb = 2kbs,    Fd = 5kHz
    GFSK_Rb2_4Fd36,      ///< GFSK, No Manchester, Rb = 2.4kbs,  Fd = 36kHz
    GFSK_Rb4_8Fd45,      ///< GFSK, No Manchester, Rb = 4.8kbs,  Fd = 45kHz
    GFSK_Rb9_6Fd45,      ///< GFSK, No Manchester, Rb = 9.6kbs,  Fd = 45kHz
    GFSK_Rb19_2Fd9_6,    ///< GFSK, No Manchester, Rb = 19.2kbs, Fd = 9.6kHz
    GFSK_Rb38_4Fd19_6,   ///< GFSK, No Manchester, Rb = 38.4kbs, Fd = 19.6kHz
    GFSK_Rb57_6Fd28_8,   ///< GFSK, No Manchester, Rb = 57.6kbs, Fd = 28.8kHz
    GFSK_Rb125Fd125,     ///< GFSK, No Manchester, Rb = 125kbs,  Fd = 125kHz

    OOK_Rb1_2Bw75,       ///< OOK, No Manchester, Rb = 1.2kbs,  Rx Bandwidth = 75kHz
    OOK_Rb2_4Bw335,      ///< OOK, No Manchester, Rb = 2.4kbs,  Rx Bandwidth = 335kHz
    OOK_Rb4_8Bw335,      ///< OOK, No Manchester, Rb = 4.8kbs,  Rx Bandwidth = 335kHz
    OOK_Rb9_6Bw335,      ///< OOK, No Manchester, Rb = 9.6kbs,  Rx Bandwidth = 335kHz
    OOK_Rb19_2Bw335,     ///< OOK, No Manchester, Rb = 19.2kbs, Rx Bandwidth = 335kHz
    OOK_Rb38_4Bw335,     ///< OOK, No Manchester, Rb = 38.4kbs, Rx Bandwidth = 335kHz
    OOK_Rb40Bw335        ///< OOK, No Manchester, Rb = 40kbs,   Rx Bandwidth = 335kHz
    } ModemConfigChoice;

    /// Constructor. You can have multiple instances, but each instance must have its own
    /// interrupt and slave select pin. After constructing, you must call init() to initialise the intnerface
    /// and the radio module
    /// \param[in] slaveSelectPin the Arduino pin number of the output to use to select the RF22 before
    /// accessing it. Defaults to the normal SS pin for your Arduino (D10 for Diecimila, Uno etc, D53 for Mega)
    /// \param[in] interrupt The interrupt number to use. Default is interrupt 0 (Arduino input pin 2)
    RF22(PinName slaveSelectPin , PinName mosi, PinName miso, PinName sclk, PinName interrupt );
  
    /// Initialises this instance and the radio module connected to it.
    /// The following steps are taken:
    /// - Initialise the slave select pin and the SPI interface library
    /// - Software reset the RF22 module
    /// - Checks the connected RF22 module is either a RF22_DEVICE_TYPE_RX_TRX or a RF22_DEVICE_TYPE_TX
    /// - Attaches an interrupt handler
    /// - Configures the RF22 module
    /// - Sets the frequncy to 434.0 MHz
    /// - Sets the modem data rate to FSK_Rb2_4Fd36
    /// \return  true if everything was successful
    boolean        init();

    /// Issues a software reset to the 
    /// RF22 module. Blocks for 1ms to ensure the reset is complete.
    void           reset();

    /// Reads a single register from the RF22
    /// \param[in] reg Register number, one of RF22_REG_*
    /// \return The value of the register
    uint8_t        spiRead(uint8_t reg);

    /// Writes a single byte to the RF22
    /// \param[in] reg Register number, one of RF22_REG_*
    /// \param[in] val The value to write
    void           spiWrite(uint8_t reg, uint8_t val);

    /// Reads a number of consecutive registers from the RF22 using burst read mode
    /// \param[in] reg Register number of the first register, one of RF22_REG_*
    /// \param[in] dest Array to write the register values to. Must be at least len bytes
    /// \param[in] len Number of bytes to read
    void           spiBurstRead(uint8_t reg, uint8_t* dest, uint8_t len);

    /// Write a number of consecutive registers using burst write mode
    /// \param[in] reg Register number of the first register, one of RF22_REG_*
    /// \param[in] src Array of new register values to write. Must be at least len bytes
    /// \param[in] len Number of bytes to write
    void           spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len);

    /// Reads and returns the device status register RF22_REG_02_DEVICE_STATUS
    /// \return The value of the device status register
    uint8_t        statusRead();
  
    /// Reads a value from the on-chip analog-digital converter
    /// \param[in] adcsel Selects the ADC input to measure. One of RF22_ADCSEL_*. Defaults to the 
    /// internal temperature sensor
    /// \param[in] adcref Specifies the refernce voltage to use. One of RF22_ADCREF_*. 
    /// Defaults to the internal bandgap voltage.
    /// \param[in] adcgain Amplifier gain selection. 
    /// \param[in] adcoffs Amplifier offseet (0 to 15).
    /// \return The analog value. 0 to 255.
    uint8_t        adcRead(uint8_t adcsel = RF22_ADCSEL_INTERNAL_TEMPERATURE_SENSOR,
               uint8_t adcref = RF22_ADCREF_BANDGAP_VOLTAGE,
               uint8_t adcgain = 0, 
               uint8_t adcoffs = 0);

    /// Reads the on-chip temperature sensoer
    /// \param[in] tsrange Specifies the temperature range to use. One of RF22_TSRANGE_*
    /// \param[in] tvoffs Specifies the temperature value offset. This is actually signed value 
    /// added to the measured temperature value
    /// \return The measured temperature.
    uint8_t        temperatureRead(uint8_t tsrange = RF22_TSRANGE_M64_64C, uint8_t tvoffs = 0);   

    /// Reads the wakeup timer value in registers RF22_REG_17_WAKEUP_TIMER_VALUE1 
    /// and RF22_REG_18_WAKEUP_TIMER_VALUE2
    /// \return The wakeup timer value 
    uint16_t       wutRead();

    /// Sets the wakeup timer period registers RF22_REG_14_WAKEUP_TIMER_PERIOD1,
    /// RF22_REG_15_WAKEUP_TIMER_PERIOD2 and RF22_R<EG_16_WAKEUP_TIMER_PERIOD3
    /// \param[in] wtm Wakeup timer mantissa value
    /// \param[in] wtr Wakeup timer exponent R value
    /// \param[in] wtd Wakeup timer exponent D value
    void           setWutPeriod(uint16_t wtm, uint8_t wtr = 0, uint8_t wtd = 0);

    /// Sets the transmitter and receiver centre frequency
    /// \param[in] centre Frequency in MHz. 240.0 to 960.0. Caution, some versions of RF22 and derivatives 
    /// implemented more restricted frequency ranges.
    /// \param[in] afcPullInRange Sets the AF Pull In Range in MHz. Defaults to 0.05MHz (50kHz). Range is 0.0 to 0.159375
    /// for frequencies 240.0 to 480MHz, and 0.0 to 0.318750MHz for  frequencies 480.0 to 960MHz, 
    /// \return true if the selected frquency centre + (fhch * fhs) is within range and the afcPullInRange is within range
    boolean        setFrequency(float centre, float afcPullInRange = 0.05);

    /// Sets the frequency hopping step size.
    /// \param[in] fhs Frequency Hopping step size in 10kHz increments
    /// \return true if centre + (fhch * fhs) is within limits
    boolean        setFHStepSize(uint8_t fhs);

    /// Sets the frequncy hopping channel. Adds fhch * fhs to centre frequency
    /// \param[in] fhch The channel number
    /// \return true if the selected frquency centre + (fhch * fhs) is within range
    boolean        setFHChannel(uint8_t fhch);

    /// Reads and returns the current RSSI value from register RF22_REG_26_RSSI. If you want to find the RSSI
    /// of the last received message, use lastRssi() instead.
    /// \return The current RSSI value 
    uint8_t        rssiRead();

    /// Reads and returns the current EZMAC value from register RF22_REG_31_EZMAC_STATUS
    /// \return The current EZMAC value
    uint8_t        ezmacStatusRead();

    /// Sets the parameters for the RF22 Idle mode in register RF22_REG_07_OPERATING_MODE. 
    /// Idle mode is the mode the RF22 will be in when not transmitting or receiving. The default idle mode 
    /// is RF22_XTON ie READY mode. 
    /// \param[in] mode Mask of mode bits, using RF22_SWRES, RF22_ENLBD, RF22_ENWT, 
    /// RF22_X32KSEL, RF22_PLLON, RF22_XTON.
    void           setMode(uint8_t mode);

    /// If current mode is Rx or Tx changes it to Idle. If the transmitter or receiver is running, 
    /// disables them.
    void           setModeIdle();

    /// If current mode is Tx or Idle, changes it to Rx. 
    /// Starts the receiver in the RF22.
    void           setModeRx();

    /// If current mode is Rx or Idle, changes it to Rx. 
    /// Starts the transmitter in the RF22.
    void           setModeTx();

    /// Returns the operating mode of the library.
    /// \return the current mode, one of RF22_MODE_*
    uint8_t        mode();

    /// Sets the transmitter power output level in register RF22_REG_6D_TX_POWER.
    /// Be a good neighbour and set the lowest power level you need.
    /// After init(), the power wil be set to RF22_TXPOW_8DBM.
    /// Caution: In some countries you may only select RF22_TXPOW_17DBM if you
    /// are also using frequency hopping.
    /// \param[in] power Transmitter power level, one of RF22_TXPOW_*
    void           setTxPower(uint8_t power);

    /// Sets all the registered required to configure the data modem in the RF22, including the data rate, 
    /// bandwidths etc. You cas use this to configure the modem with custom configuraitons if none of the 
    /// canned configurations in ModemConfigChoice suit you.
    /// \param[in] config A ModemConfig structure containing values for the modem configuration registers.
    void           setModemRegisters(const ModemConfig* config);

    /// Select one of the predefined modem configurations. If you need a modem configuration not provided 
    /// here, use setModemRegisters() with your own ModemConfig.
    /// \param[in] index The configuration choice.
    /// \return true if index is a valid choice.
    boolean        setModemConfig(ModemConfigChoice index);

    /// Starts the receiver and checks whether a received message is available.
    /// This can be called multiple times in a timeout loop
    /// \return true if a complete, valid message has been received and is able to be retrieved by
    /// recv()
    boolean        available();

    /// Starts the receiver and blocks until a valid received 
    /// message is available.
    void           waitAvailable();

    /// Starts the receiver and blocks until a received message is available or a timeout
    /// \param[in] timeout Maximum time to wait in milliseconds.
    /// \return true if a message is available
    bool           waitAvailableTimeout(uint16_t timeout);

    /// Turns the receiver on if it not already on.
    /// If there is a valid message available, copy it to buf and return true
    /// else return false.
    /// If a message is copied, *len is set to the length (Caution, 0 length messages are permitted).
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received message
    /// \param[in,out] len Pointer to available space in buf. Set to the actual number of octets copied.
    /// \return true if a valid message was copied to buf
    boolean        recv(uint8_t* buf, uint8_t* len);

    /// Waits until any previous transmit packet is finished being transmitted with waitPacketSent().
    /// Then loads a message into the transmitter and starts the transmitter. Note that a message length
    /// of 0 is NOT permitted. 
    /// \param[in] data Array of data to be sent
    /// \param[in] len Number of bytes of data to send (> 0)
    /// \return true if the message length was valid and it was correctly queued for transmit
    boolean        send(const uint8_t* data, uint8_t len);

    /// Blocks until the RF22 is not in mode RF22_MODE_TX (ie until the RF22 is not transmitting).
    /// This effectively waits until any previous transmit packet is finished being transmitted.
    void           waitPacketSent();
  
    /// Tells the receiver to accept messages with any TO address, not just messages
    /// addressed to this node or the broadcast address
    /// \param[in] promiscuous true if you wish to receive messages with any TO address
    void           setPromiscuous(boolean promiscuous);

    /// Returns the TO header of the last received message
    /// \return The TO header
    uint8_t        headerTo();

    /// Returns the FROM header of the last received message
    /// \return The FROM header
    uint8_t        headerFrom();

    /// Returns the ID header of the last received message
    /// \return The ID header
    uint8_t        headerId();

    /// Returns the FLAGS header of the last received message
    /// \return The FLAGS header
    uint8_t        headerFlags();

    /// Returns the RSSI (Receiver Signal Strength Indicator)
    /// of the last received message. This measurement is taken when 
    /// the preamble has been received. It is a (non-linear) measure of the received signal strength.
    /// \return The RSSI
    uint8_t        lastRssi();

    /// Prints a data buffer in HEX.
    /// For diagnostic use
    /// \param[in] prompt string to preface the print
    /// \param[in] buf Location of the buffer to print
    /// \param[in] len Length of the buffer in octets.
    static void           printBuffer(const char* prompt, const uint8_t* buf, uint8_t len);

    /// Sets the length of the preamble
    /// in 4-bit nibbles. 
    /// Caution: this should be set to the same 
    /// value on all nodes in your network. Default is 8.
    /// Sets the message preamble length in RF22_REG_34_PREAMBLE_LENGTH
    /// \param[in] nibbles Preamble length in nibbles of 4 bits each.  
    void           setPreambleLength(uint8_t nibbles);

    /// Sets the sync words for transmit and receive in registers RF22_REG_36_SYNC_WORD3 
    /// to RF22_REG_39_SYNC_WORD0
    /// Caution: this should be set to the same 
    /// value on all nodes in your network. Default is { 0x2d, 0xd4 }
    /// \param[in] syncWords Array of sync words
    /// \param[in] len Number of sync words to set
    void           setSyncWords(const uint8_t* syncWords, uint8_t len);

protected:
    /// This is a low level function to handle the interrupts for one instance of RF22.
    /// Called automatically by isr0() and isr1()
    /// Should not need to be called.
    void           handleInterrupt();

    /// Clears the receiver buffer.
    /// Internal use only
    void           clearRxBuf();

    /// Clears the transmitter buffer
    /// Internal use only
    void           clearTxBuf();

    /// Fills the transmitter buffer with the data of a mesage to be sent
    /// \param[in] data Array of data bytes to be sent (1 to 255)
    /// \param[in] len Number of data bytes in data (> 0)
    /// \return true if the message length is valid
    boolean           fillTxBuf(const uint8_t* data, uint8_t len);

    /// Appends the transmitter buffer with the data of a mesage to be sent
    /// \param[in] data Array of data bytes to be sent (0 to 255)
    /// \param[in] len Number of data bytes in data
    /// \return false if the resulting message would exceed RF22_MAX_MESSAGE_LEN, else true
    boolean           appendTxBuf(const uint8_t* data, uint8_t len);

    /// Internal function to load the next fragment of 
    /// the current message into the transmitter FIFO
    /// Internal use only
    void           sendNextFragment();

    ///  function to copy the next fragment from 
    /// the receiver FIF) into the receiver buffer
    void           readNextFragment();

    /// Clears the RF22 Rx and Tx FIFOs
    /// Internal use only
    void           resetFifos();

    /// Clears the RF22 Rx FIFO
    /// Internal use only
    void           resetRxFifo();

    /// Clears the RF22 Tx FIFO
    /// Internal use only
    void           resetTxFifo();

    /// This function will be called by handleInterrupt() if an RF22 external interrupt occurs. 
    /// This can only happen if external interrupts are enabled in the RF22 
    /// (which they are not by default). 
    /// Subclasses may override this function to get control when  an RF22 external interrupt occurs. 
    virtual void   handleExternalInterrupt();

    /// This function will be called by handleInterrupt() if an RF22 wakeup timer interrupt occurs. 
    /// This can only happen if wakeup timer interrupts are enabled in the RF22 
    /// (which they are not by default). 
    /// Subclasses may override this function to get control when  an RF22 wakeup timer interrupt occurs. 
    virtual void   handleWakeupTimerInterrupt();

    /// Sets the TO header to be sent in all subsequent messages
    /// \param[in] to The new TO header value
    void           setHeaderTo(uint8_t to);

    /// Sets the FROM header to be sent in all subsequent messages
    /// \param[in] from The new FROM header value
    void           setHeaderFrom(uint8_t from);

    /// Sets the ID header to be sent in all subsequent messages
    /// \param[in] id The new ID header value
    void           setHeaderId(uint8_t id);

    /// Sets the FLAGS header to be sent in all subsequent messages
    /// \param[in] flags The new FLAGS header value
    void           setHeaderFlags(uint8_t flags);

    /// Start the transmission of the contents 
    /// of the Tx buffer
    void           startTransmit();

    /// ReStart the transmission of the contents 
    /// of the Tx buffer after a atransmission failure
    void           restartTransmit();

protected:
    //GenericSPIClass*    _spi;

    /// Low level interrupt service routine for RF22 connected to interrupt 0
    void         isr0();

    /// Low level interrupt service routine for RF22 connected to interrupt 1
    //static void         isr1();
private:    
   
    volatile uint8_t    _mode; // One of RF22_MODE_*

    uint8_t             _sleepMode;
    uint8_t             _idleMode;
    DigitalOut          _slaveSelectPin;
    SPI                 _spi;
    InterruptIn         _interrupt;
    uint8_t             _deviceType;

    // These volatile members may get changed in the interrupt service routine
    volatile uint8_t    _bufLen;
    uint8_t             _buf[RF22_MAX_MESSAGE_LEN];

    volatile boolean    _rxBufValid;

    volatile boolean    _txPacketSent;
    volatile uint8_t    _txBufSentIndex;
  
    volatile uint16_t   _rxBad;
    volatile uint16_t   _rxGood;
    volatile uint16_t   _txGood;

    volatile uint8_t    _lastRssi;
};


#endif 
