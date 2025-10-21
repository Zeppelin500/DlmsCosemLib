# DlmsCosemLib - an Arduino P1 DLMS/COSEM Decoder Library

[![version](https://img.shields.io/badge/version-0.1.0-brightgreen.svg)](CHANGELOG.md)
[![license](https://img.shields.io/badge/license-GPL--3.0-orange.svg)](LICENSE)


## Documentation

The **DlmsCosemLib** is a Arduino decoder Library for austrian el. energy meters with the P1 DLMS/COSEM interface.

Input is the already decrypted payload plaintext of the M-Bus message. The decoded records will be returned as a JSON.


Live test of the [**DlmsCosemLib Example**](https://wokwi.com/projects/445347018581022721) at wokwi.com


A working P1 --> MQTT gateway with this library [**MBusinoP1**](https://github.com/Zeppelin500/MBusinoP1)

### Credits

Thanks to **DomiStyle** for https://github.com/DomiStyle/esphome-dlms-meter where some ideas for decoding come from.

Thanks to **HWHardsoft** for the code base of this Library https://github.com/HWHardsoft/DLMS-MBUS-Reader

### Class: `DlmsCosemLib`

Include and instantiate the DlmsCosemLib class. 

```c
#include <DlmsCosemLib.h>

DlmsCosemLib dlmsCosem;
```


## Decoding

### Method: `decode`

Decodes an already decrypted payload plaintext of the M-Bus message. The decoded records will be written as a JsonArray (requires ArduinoJson library). The result is an array of objects. The method call returns the number of decoded fields or negative numbers if error.

```c
uint8_t decode(uint8_t *plaintext, uint8_t size, JsonArray& root);
```

same line from the example
```c
fields = DlmsCosem.decode(&plaintext[0], sizeof(plaintext), root);
```

Example JSON output:

```
[
{
    "value_scaled": 22.06,
    "units": "C",
    "name": "external_temperature_min"
  },
]
```

Example extract the JSON

```c    
      for (uint8_t i = 1; i < fields; i++) {
        const char *obisString = root[i]["obis"];
        const char *name = root[i]["name"];
        const char *units = root[i]["units"];
        double value = root[i]["value_scaled"].as<double>();
        
        send or process the records    
      }
```
You will find more details in the example
 
### possible contained records
only contained records will be sended

* **["obisString"]** contains the OBIS Code as ASCII array.
* **["value_scaled"]** contains the value of the record as 64 bit real
* **["units"]** contains the unit of the value as ASCII array
* **["name"]** contains the name of the value as ASCII array
* **["meterNumber"]** countains the serial Number of the meter
* **["timestamp"]** countains the timestamp of the records

There are more records available but you have to delete the out comment in the library.



### Method: `getError`

Returns the last error as ASCII array.

* `Unsupported OBIS header type`
* `Unsupported OBIS header length`
* `Unsupported OBIS header medium`
* `Unsupported OBIS data type`


```c
const char * getError(int8_t code);
```

### Home Assistant

The returned units are Home Assistant compatible. 
There are two methods to get the right device- and state-classes for use as Home Assistant sensor.

```c
const char* getDeviceClass(uint8_t code);
```

```c
const char* getStateClass(uint8_t code);
```


## References

* [Netz NÖ P1 Kundenschnittstelle](https://www.netz-noe.at/Download-(1)/Smart-Meter/218_9_SmartMeter_Kundenschnittstelle_lektoriert_14.aspx)


## License


The DlmsCosemLib library is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The DlmsCosemLib library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the MBusinoLib library.  If not, see <http://www.gnu.org/licenses/>.
