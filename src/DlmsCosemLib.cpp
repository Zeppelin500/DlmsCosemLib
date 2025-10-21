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

#include "DlmsCosemLib.h"

int8_t DlmsCosemLib::decode(uint8_t* plaintext, uint8_t payload_length, JsonArray& root){   // return value is the number of decodet records or the failure code if negative

  uint16_t current_position = DECODER_START_OFFSET;

  char obisString[21] = {0}; //Obis code
  char obisName[21] = {0}; //Obis name 
  char obisUnit[10] = {0}; //Unit
  char jsonTimestamp[21] = {0};
  bool timestampAvailable = false;

  uint8_t count = 0;
  do {
    count ++;
    if (plaintext[current_position + OBIS_TYPE_OFFSET] != DATA_OCTET_STRING) {
      // Unsupported OBIS header type!");
      return -1;
    }

    uint8_t data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];

    if (data_length != 0x06) {
      // read timestamp
      if ((data_length == 0x0C) && (current_position == DECODER_START_OFFSET)) {
        uint8_t dateTime[data_length];
        memcpy(&dateTime[0], &plaintext[current_position + 2], data_length);

        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;

        year = (plaintext[current_position + 2] << 8) + plaintext[current_position + 3];
        month = plaintext[current_position + 4];
        day = plaintext[current_position + 5];
        hour = plaintext[current_position + 7];
        minute = plaintext[current_position + 8];
        second = plaintext[current_position + 9];

        char timeStamp[21];
        sprintf(timeStamp, "%02u.%02u.%04u %02u:%02u:%02u", day, month, year, hour, minute, second);

        strlcpy(jsonTimestamp, timeStamp,21);
        timestampAvailable = true;        

        current_position = 34;
        data_length = plaintext[current_position + OBIS_LENGTH_OFFSET];
      } else if ((data_length == 0x0C) && (current_position > 225)) {
        char meterNumber[data_length+ 1] = {0};
        memcpy(&meterNumber[0], &plaintext[current_position + 2], data_length);
        
        JsonObject data = root.add<JsonObject>();
        data["meter_number"] = String(meterNumber);

        // THIS IS THE END OF THE PACKET
        break;
      } else {
        return -2;//  Unsupported OBIS header length");
      }
    }

    uint8_t obis_code[data_length];
    memcpy(&obis_code[0], &plaintext[current_position + OBIS_CODE_OFFSET], data_length);  // Copy OBIS code to array

    current_position += data_length + 2;  // Advance past code, position and type

    uint8_t obis_data_type = plaintext[current_position];
    current_position++;  // Advance past data type

    uint8_t obis_data_length = 0x00;
    uint8_t  code = TYPE_UNKNOWN;   // Library internal type code 

    if (obis_code[OBIS_A] == 0x01) {
      // Compare C and D against code
      if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L1, 2) == 0) {
         code = TYPE_VOLTAGE_L1;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L2, 2) == 0) {
         code = TYPE_VOLTAGE_L2;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_VOLTAGE_L3, 2) == 0) {
         code = TYPE_VOLTAGE_L3;
      }

      else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L1, 2) == 0) {
         code = TYPE_CURRENT_L1;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L2, 2) == 0) {
         code = TYPE_CURRENT_L2;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_CURRENT_L3, 2) == 0) {
         code = TYPE_CURRENT_L3;
      }

      else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_POWER_PLUS, 2) == 0) {
         code = TYPE_ACTIVE_POWER_PLUS;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_POWER_MINUS, 2) == 0) {
         code = TYPE_ACTIVE_POWER_MINUS;
      }

      else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_ENERGY_PLUS, 2) == 0) {
         code = TYPE_ACTIVE_ENERGY_PLUS;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_ACTIVE_ENERGY_MINUS, 2) == 0) {
         code = TYPE_ACTIVE_ENERGY_MINUS;
      }

      else if (memcmp(&obis_code[OBIS_C], OBIS_REACTIVE_ENERGY_PLUS, 2) == 0) {
         code = TYPE_REACTIVE_ENERGY_PLUS;
      } else if (memcmp(&obis_code[OBIS_C], OBIS_REACTIVE_ENERGY_MINUS, 2) == 0) {
         code = TYPE_REACTIVE_ENERGY_MINUS;
      }  

      else if (memcmp(&obis_code[OBIS_C], OBIS_POWER_FACTOR, 2) == 0) {
         code = TYPE_POWER_FACTOR;
      } else {
        // Unsupported OBIS code");
      }
    } else {
      
      return -3;// Unsupported OBIS medium");
    }

    uint16_t uint16_value;
    uint32_t uint32_value;
    float float_value;

    switch (obis_data_type) {
      case DATA_LONG_DOUBLE_UNSIGNED:
        obis_data_length = 4;

        memcpy(&uint32_value, &plaintext[current_position], 4);  // Copy uint8_ts to integer
        uint32_value = _swap_uint32(uint32_value);                // Swap uint8_ts

        float_value = uint32_value;  // Ignore decimal digits for now
        break;

      case DATA_LONG_UNSIGNED:
        obis_data_length = 2;

        memcpy(&uint16_value, &plaintext[current_position], 2);  // Copy uint8_ts to integer
        uint16_value = _swap_uint16(uint16_value);                // Swap uint8_ts

        if (plaintext[current_position + 5] == SCALE_TENTHS)
          float_value = uint16_value / 10.0;
        else if (plaintext[current_position + 5] == SCALE_HUNDREDTHS)
          float_value = uint16_value / 100.0;
        else if (plaintext[current_position + 5] == SCALE_THOUSANDS)
          float_value = uint16_value / 1000.0;
        else
          float_value = uint16_value;
        
        break;

      case DATA_OCTET_STRING:
        obis_data_length = plaintext[current_position];
        current_position++;  // Advance past string length
        break;

      default:
        return -4;         // Unsupported OBIS data type
        break;
    }
    
    // Init JSON object
    
    if(timestampAvailable == true){   // Add the Timestamp before the first record
      JsonObject data = root.add<JsonObject>();
      timestampAvailable = false;
      data["timestamp"] = String(jsonTimestamp);
    }
    JsonObject data = root.add<JsonObject>();
    data["code"] =  code;
    data["obis"] = String(_getObisCode(code));
    data["name"] = String(_getCodeName(code));
    data["value_scaled"] = float_value;  
    data["units"] = String(_getCodeUnits(code)); 


    current_position += obis_data_length;  // Skip data length

    current_position += 2;  // Skip pause after data

    if (plaintext[current_position] == 0x0F){     // There is still additional data for this type, skip it
      current_position += 4; // Skip additional data and additional break; this will jump out of bounds on last frame
    }  
    if(current_position >= payload_length){
      count++;
    }                  
  } while (current_position < payload_length);  // Loop until end of packet

  return count;
}

uint16_t DlmsCosemLib::_swap_uint16(uint16_t val) {
  return (val << 8) | (val >> 8);
}

uint32_t DlmsCosemLib::_swap_uint32(uint32_t val) {
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | (val >> 16);
}

const char * DlmsCosemLib::getError(int8_t code) {
  switch (code) {

    case UNSUPPORTED_OBIS_HEADER:
      return "Unsupported OBIS header type";
    
    case UNSUPPORTED_OBIS_HEADER_LENGTH:
      return "Unsupported OBIS header length";

    case UNSUPPORTED_OBIS_HEADER_MEDIUM: 
      return "Unsupported OBIS header medium";

    case UNSUPPORTED_OBIS_DATA_TYPE: 
      return "Unsupported OBIS data type";

    default:
      break; 
  }
}

const char * DlmsCosemLib::_getObisCode(uint8_t code) {
  switch (code) {

    case TYPE_UNKNOWN:
      return "TypeUnknown"; 
      
    case TYPE_VOLTAGE_L1:
      return "1.0.32.7.0";    

    case TYPE_VOLTAGE_L2:
      return "1.0.52.7.0";    

    case TYPE_VOLTAGE_L3:
      return "1.0.72.7.0";

    case TYPE_CURRENT_L1:   
      return "1.0.31.7.0";    

    case TYPE_CURRENT_L2:  
      return "1.0.51.7.0";   

    case TYPE_CURRENT_L3: 
      return "1.0.71.7.0";

    case TYPE_POWER_FACTOR:  
      return "1.0.13.7.0";        
 
    case TYPE_ACTIVE_ENERGY_PLUS:  
      return "1.0.1.8.0";

    case TYPE_ACTIVE_ENERGY_MINUS: 
      return "1.0.2.8.0";

    case TYPE_REACTIVE_ENERGY_PLUS:  
      return "1.0.3.8.0";

    case TYPE_REACTIVE_ENERGY_MINUS: 
      return "1.0.4.8.0";      

    case TYPE_ACTIVE_POWER_PLUS:  
      return "1.0.1.7.0";        

    case TYPE_ACTIVE_POWER_MINUS:  
      return "1.0.2.7.0";      

    default:
        break;

  }
  return "";
}

const char * DlmsCosemLib::_getCodeName(uint8_t code) {
  switch (code) {

    case TYPE_UNKNOWN:
      return "TypeUnknown"; 
      
    case TYPE_VOLTAGE_L1:
      return "VoltageL1";    

    case TYPE_VOLTAGE_L2:
      return "VoltageL2";    

    case TYPE_VOLTAGE_L3:
      return "VoltageL3";

    case TYPE_CURRENT_L1:   
      return "CurrentL1";    

    case TYPE_CURRENT_L2:  
      return "CurrentL2";   

    case TYPE_CURRENT_L3: 
      return "CurrentL3";

    case TYPE_POWER_FACTOR:  
      return "PowerFactor";        
 
    case TYPE_ACTIVE_ENERGY_PLUS:  
      return "ActiveEnergyPlus";

    case TYPE_ACTIVE_ENERGY_MINUS: 
      return "ActiveEnergyMinus";

    case TYPE_REACTIVE_ENERGY_PLUS:  
      return "ReactiveEnergyPlus";

    case TYPE_REACTIVE_ENERGY_MINUS: 
      return "ReactiveEnergyMinus";  

    case TYPE_ACTIVE_POWER_PLUS:  
      return "ActivePowerPlus";        

    case TYPE_ACTIVE_POWER_MINUS:  
      return "ActivePowerMinus";      

    default:
        break;

  }
  return "";
}

const char * DlmsCosemLib::_getCodeUnits(uint8_t code) {
  switch (code) {

    case TYPE_ACTIVE_ENERGY_PLUS:  
    case TYPE_ACTIVE_ENERGY_MINUS: 
      return "Wh";

    case TYPE_REACTIVE_ENERGY_PLUS:  
    case TYPE_REACTIVE_ENERGY_MINUS: 
      return "varh";      // varh is not HA compatible until now

    case TYPE_ACTIVE_POWER_PLUS:    
    case TYPE_ACTIVE_POWER_MINUS:  
      return "W";
      
    case TYPE_VOLTAGE_L1:
    case TYPE_VOLTAGE_L2:
    case TYPE_VOLTAGE_L3:
      return "V";

    case TYPE_CURRENT_L1:     
    case TYPE_CURRENT_L2:             
    case TYPE_CURRENT_L3: 
      return "A";

    case TYPE_POWER_FACTOR:          
    case TYPE_UNKNOWN:
      return "";  
    default:
        break;

  }
  return "";
}

const char * DlmsCosemLib::getDeviceClass(uint8_t code) {
  switch (code) {

    case TYPE_ACTIVE_ENERGY_PLUS:  
    case TYPE_ACTIVE_ENERGY_MINUS:
    case TYPE_REACTIVE_ENERGY_PLUS: // until now, there is no device class reactive energy in HA
    case TYPE_REACTIVE_ENERGY_MINUS:  
      return "energy";
    
    case TYPE_POWER_FACTOR:   
      return "power_factor";

    case TYPE_ACTIVE_POWER_PLUS:    
    case TYPE_ACTIVE_POWER_MINUS: 
      return "power";
      
    case TYPE_VOLTAGE_L1:
    case TYPE_VOLTAGE_L2:
    case TYPE_VOLTAGE_L3:
      return "voltage";

    case TYPE_CURRENT_L1:     
    case TYPE_CURRENT_L2:             
    case TYPE_CURRENT_L3: 
      return "current";
    
    case TYPE_UNKNOWN:
      return "";  
    default:
        break;

  }
  return "";
}

const char * DlmsCosemLib::getStateClass(uint8_t code) {
  switch (code) {

    case TYPE_UNKNOWN:
    case TYPE_VOLTAGE_L1:
    case TYPE_VOLTAGE_L2:
    case TYPE_VOLTAGE_L3:
    case TYPE_CURRENT_L1:     
    case TYPE_CURRENT_L2:             
    case TYPE_CURRENT_L3:             
    case TYPE_POWER_FACTOR:           
    case TYPE_ACTIVE_POWER_PLUS:    
    case TYPE_ACTIVE_POWER_MINUS:      
      return "measurement";

case TYPE_ACTIVE_ENERGY_PLUS:  
case TYPE_ACTIVE_ENERGY_MINUS:   
    case TYPE_REACTIVE_ENERGY_PLUS: 
    case TYPE_REACTIVE_ENERGY_MINUS: 
      return "total";

    default:
        break;
  }
  return "";
}

