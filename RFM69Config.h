#ifndef RFM69Config_h
#define RFM69Config_h

#include "RFM69.h"

/*PROGMEM */ static const uint8_t CONFIG[][2] =
{
    { RFM69_REG_01_OPMODE,      RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RFM69_MODE_SLEEP },
    { RFM69_REG_02_DATA_MODUL,  RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 },
    
    { RFM69_REG_03_BITRATE_MSB, 0x3E}, // 2000 bps
    { RFM69_REG_04_BITRATE_LSB, 0x80},
    
    { RFM69_REG_05_FDEV_MSB,    0x00}, // 3000 hz (6000 hz shift)
    { RFM69_REG_06_FDEV_LSB,    0x31},

    { RFM69_REG_07_FRF_MSB,     0xD9 }, // 869.5 MHz
    { RFM69_REG_08_FRF_MID,     0x80 },
    { RFM69_REG_09_FRF_LSB,     0x12 },
    
    // PA Settings
    // +20dBm formula: Pout=-11+OutputPower[dBmW] (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
    // Without extra flags: Pout=-14+OutputPower[dBmW]
    { RFM69_REG_11_PA_LEVEL,    RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON | 0x18},  // 10mW
    //{ REG_PALEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON | 0x1f},// 50mW
    
    { RFM69_REG_13_OCP,         RF_OCP_ON | RF_OCP_TRIM_95 },
    
    //{ RFM69_REG_18_LNA,       RF_LNA_ZIN_50 }, // Default is 200 ohm?
    
    { RFM69_REG_19_RX_BW,       RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} // Rx Bandwidth: 10.4kHz
    { RFM69_REG_25_DIO_MAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 RX: PayloadReady TX: TXReady
    
    // { RFM69_REG_2D_PREAMBLE_LSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
    
    { RFM69_REG_2E_SYNC_CONFIG, RF_SYNC_OFF }, // Sync bytes off
    { RFM69_REG_37_PACKET_CONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
    { RFM69_REG_38_PAYLOAD_LENGTH, 66 }, // Full FIFO size for rx packet
    { RFM69_REG_3C_FIFO_THRESHOLD, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, //TX on FIFO not empty
    { RFM69_REG_3D_PACKET_CONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, //RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    { RFM69_REG_6F_TEST_DAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode, recommended default for AfcLowBetaOn=0
    {255, 0}
  };

#endif
