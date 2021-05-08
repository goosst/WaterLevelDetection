# WaterLevelDetection

## Goal

Measure the level of available water in my rainwater well.

## Wishlist

- Report status through WiFi (mqtt-messages) to integrate in the rest of my home automation 
- Since waterlevel only needs to be measured every x hours or days, be able to use a deepsleep method to safe power
- Use standard available components (through hole components for electronics, simple sensors, ... )
- Over the air updates

## Concept and used components

To take care of the Wifi and mqtt messages, an ESP8266 is used ([Wemos D1 mini](https://www.banggood.com/Geekcreit-D1-mini-V2_2_0-WIFI-Internet-Development-Board-Based-ESP8266-4MB-FLASH-ESP-12S-Chip-p-1143874.html?cur_warehouse=CN&rmmds=search&p=ET150713234951201708&custlinkid=1551683)).

The used measuring principle is based on water pressure, since the pressure at the bottom of the well is proportional with the height of the water. Other methods using distance sensors sound quite unreliable (reflections, need to have a wide cone of free air to measure to the bottom of the well, ... ).

The sensor used here, is this one: [Pressure sensor](https://www.banggood.com/Submersible-Water-Level-Transmitter-Level-Transducer-Sensor-0-5mH2O-6m-Cable-p-1146896.html?rmmds=myorder&cur_warehouse=CN&p=ET150713234951201708&custlinkid=1551677).

This sensor creates an additional challenge, it returns a current between 4-20mA proportional with pressure and requires a 24V supply voltage. 

- To measure the 4-20 mA, an INA219 sensor is used. This is a high side current sensor and additionally measures the bus voltage, which helps us later with the dc-converter.
- I was not able to identify a boost DC-DC converter that was through-hole (not SMD) and had an enable pin. So I've decided to create one myself. It's important to use an N-channel mosfet which can be switched by a 3.3V signal from the ESP8266. Diode, inductor, capacitor are standard components which you can find cheaply.
Other attempts made with breakoutboards using the MT3608 converter, didn't really produce a stable output voltage when temperature changed etc.


| Components used      | 
| :------------- | 
|  [Wemos D1 mini](https://www.banggood.com/Geekcreit-D1-mini-V2_2_0-WIFI-Internet-Development-Board-Based-ESP8266-4MB-FLASH-ESP-12S-Chip-p-1143874.html?cur_warehouse=CN&rmmds=search&p=ET150713234951201708&custlinkid=1551683)| 
| [Pressure sensor](https://www.banggood.com/Submersible-Water-Level-Transmitter-Level-Transducer-Sensor-0-5mH2O-6m-Cable-p-1146896.html?rmmds=myorder&cur_warehouse=CN&p=ET150713234951201708&custlinkid=1551677) | 
| INA219: [example](https://nl.aliexpress.com/item/4000330275495.html?spm=a2g0s.9042311.0.0.28c74c4d10PHbJ) |
| N-channel mosfet switchable by 3.3V digital signal: example FQP30N06L |
| inductor, diode, capacitor |


## Implementation

[conceptual implementation](concept.svg)



## Calibration

| Level (meter)       | Shunt voltage mV     | shunt current mA     |
| :------------- | :----------: | -----------: |
|  0 (air) | 51   | 5.1    |
| 1   | 85 | 8.5 | 
| 1.5   | 99 | 9.9 | |


## Open issues

- clean up Kicad files (too many items on it used when debugging)