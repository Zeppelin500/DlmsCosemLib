/*

DlmsCosemLib, an Arduino DLMS/COSEM decoder for the P1 geateway of austrian electrical energy meters
Copyright 2025 Zeppelin500 

Thanks to **DomiStyle** for https://github.com/DomiStyle/esphome-dlms-meter where some ideas for decoding come from.

Thanks to **HWHardsoft** for the code base of this Library https://github.com/HWHardsoft/DLMS-MBUS-Reader

The DlmsCosemLib library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The DlmsCosemLib library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the DlmsCosemLib library.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DLMSCOSEMLIB_H
#define DLMSCOSEM_H

#include <Arduino.h>
#include <ArduinoJson.h>


#define DLMS_HEADER1_OFFSET 0 // Start of first DLMS header
#define DLMS_HEADER1_LENGTH 26

#define DLMS_HEADER2_OFFSET 256 // Start of second DLMS header
#define DLMS_HEADER2_LENGTH 9

#define DLMS_SYST_OFFSET 11 // system title
#define DLMS_SYST_LENGTH 8

#define DLMS_IC_OFFSET 22 // invocation counter
#define DLMS_IC_LENGTH 4

#define READ_TIMEOUT 2000
#define RECEIVE_BUFFER_SIZE 1024

#define MBUS_DLMS_HADER 21  // M-Bus header befor the encrypted aplication data 

static const uint8_t KEY_LENGTH = 16;

#include <inttypes.h>

// Scaling
#define SCALE_TENTHS                0xFF
#define SCALE_HUNDREDTHS            0xFE
#define SCALE_THOUSANDS             0xFD

// Data types as per specification
#define DATA_NULL                   0x00
#define DATA_OCTET_STRING           0x09
#define DATA_LONG_UNSIGNED          0x12
#define DATA_LONG_DOUBLE_UNSIGNED   0x06

#define TYPE_UNKNOWN                0
#define TYPE_VOLTAGE_L1             1
#define TYPE_VOLTAGE_L2             2
#define TYPE_VOLTAGE_L3             3
#define TYPE_CURRENT_L1             4
#define TYPE_CURRENT_L2             5
#define TYPE_CURRENT_L3             6
#define TYPE_POWER_FACTOR           7
#define TYPE_ACTIVE_POWER_PLUS      8
#define TYPE_ACTIVE_POWER_MINUS     9
#define TYPE_ACTIVE_ENERGY_PLUS     10
#define TYPE_ACTIVE_ENERGY_MINUS    11
#define TYPE_REACTIVE_ENERGY_PLUS   12
#define TYPE_REACTIVE_ENERGY_MINUS  13



// Data structure
#define DECODER_START_OFFSET 20 // Offset for start of OBIS decoding
#define OBIS_TYPE_OFFSET   0
#define OBIS_LENGTH_OFFSET 1
#define OBIS_CODE_OFFSET   2

// A.B. C.D.E.  F
// 1.0.13.7.0.255
#define OBIS_A 0
#define OBIS_B 1
#define OBIS_C 2
#define OBIS_D 3
#define OBIS_E 4
#define OBIS_F 5

// Failure Code
#define UNSUPPORTED_OBIS_HEADER  -1   //Unsupported OBIS header type
#define UNSUPPORTED_OBIS_HEADER_LENGTH -2 //Unsupported OBIS header length
#define UNSUPPORTED_OBIS_HEADER_MEDIUM -3 
#define UNSUPPORTED_OBIS_DATA_TYPE -4


// cos(phi)
static const uint8_t OBIS_POWER_FACTOR[] {
    0x0D, 0x07 // 1.0.13.7.0.255
};
// Spannung
static const uint8_t OBIS_VOLTAGE_L1[] {
    0x20, 0x07 // 1.0.32.7.0.255
};
static const uint8_t OBIS_VOLTAGE_L2[] {
    0x34, 0x07 // 1.0.52.7.0.255
};
static const uint8_t OBIS_VOLTAGE_L3[] {
    0x48, 0x07 // 1.0.72.7.0.255
};
// Strom
static const uint8_t OBIS_CURRENT_L1[] {
    0x1F, 0x07 // 1.0.31.7.0.255
};
static const uint8_t OBIS_CURRENT_L2[] {
    0x33, 0x07 // 1.0.51.7.0.255
};
static const uint8_t OBIS_CURRENT_L3[] {
    0x47, 0x07 // 1.0.71.7.0.255
};
// Momentanleistung
static const uint8_t OBIS_ACTIVE_POWER_PLUS[] {
    0x01, 0x07 // 1.0.1.7.0.255
};
static const uint8_t OBIS_ACTIVE_POWER_MINUS[] {
    0x02, 0x07 // 1.0.2.7.0.255
};
// Wirkenergie
static const uint8_t OBIS_ACTIVE_ENERGY_PLUS[] {
    0x01, 0x08 // 1.0.1.8.0.255
};
static const uint8_t OBIS_ACTIVE_ENERGY_MINUS[] {
    0x02, 0x08 // 1.0.2.8.0.255
};
static const uint8_t OBIS_REACTIVE_ENERGY_PLUS[]
{
    0x03, 0x08  // 1.0.3.8.0.255
};
static const uint8_t OBIS_REACTIVE_ENERGY_MINUS[]
{
    0x04, 0x08 // 1.0.4.8.0.255
};


class DlmsCosemLib {

public:

  int8_t decode(uint8_t* plaintext, uint8_t payload_length, JsonArray& root);
  const char * getError(int8_t code);
  const char* getDeviceClass(uint8_t code);
  const char* getStateClass(uint8_t code);

protected:

  const char * _getObisCode(uint8_t code);
  const char * _getCodeUnits(uint8_t code);
  const char * _getCodeName(uint8_t code);    
  uint32_t _swap_uint32(uint32_t val);
  uint16_t _swap_uint16(uint16_t val);


};
#endif
//------------------------------------------------------------------------

