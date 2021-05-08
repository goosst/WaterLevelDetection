# WaterLevelDetection

## Goal

Measure the level of available water in a rainwater well.

## Wishlist

- Report level of waater through WiFi (mqtt-messages), this will be further integrated in home automation 
- Since waterlevel only needs to be measured every x hours or days, be able to use a deepsleep method to safe power / prevent heating of electric components
- Use standard available components (through hole components for electronics, no SMD, ... )
- Be able to update software over the air

## Concept and used components

The used measuring principle is based on water pressure, since the pressure at the bottom of the well is proportional with the height of the water. Other methods using distance sensors sound quite unreliable (reflections, need to have a wide cone of free air to measure to the bottom of the well, ... ).

### Controller
To take care of the Wifi and mqtt messages, an ESP8266 is used ([Wemos D1 mini](https://www.banggood.com/Geekcreit-D1-mini-V2_2_0-WIFI-Internet-Development-Board-Based-ESP8266-4MB-FLASH-ESP-12S-Chip-p-1143874.html?cur_warehouse=CN&rmmds=search&p=ET150713234951201708&custlinkid=1551683)).

### Pressure sensing and power electronics
The sensor used here, is this one: [Pressure sensor](https://www.banggood.com/Submersible-Water-Level-Transmitter-Level-Transducer-Sensor-0-5mH2O-6m-Cable-p-1146896.html?rmmds=myorder&cur_warehouse=CN&p=ET150713234951201708&custlinkid=1551677).

This sensor creates an additional challenge, it returns a current between 4-20mA proportional with the pressure and requires a 24V supply voltage. 

- To measure the 4-20 mA, an INA219 sensor is used. This is a high side current sensor and additionally measures the bus voltage, which helps us later with generating a reliable supply voltage.
- I was not able to identify a boost DC-DC converter that was through-hole (not SMD) and had an enable pin. So I've decided to create one myself. It's important to use an N-channel mosfet which can be switched by a 3.3V signal from the ESP8266. Diode, inductor, capacitor are standard components which you can find cheaply.
Other attempts made with breakoutboards using the MT3608 converter, didn't really produce a stable output voltage when temperature changed etc.

### Main components
| Components used      | 
| :------------- | 
|  [Wemos D1 mini](https://www.banggood.com/Geekcreit-D1-mini-V2_2_0-WIFI-Internet-Development-Board-Based-ESP8266-4MB-FLASH-ESP-12S-Chip-p-1143874.html?cur_warehouse=CN&rmmds=search&p=ET150713234951201708&custlinkid=1551683)| 
| [Pressure sensor](https://www.banggood.com/Submersible-Water-Level-Transmitter-Level-Transducer-Sensor-0-5mH2O-6m-Cable-p-1146896.html?rmmds=myorder&cur_warehouse=CN&p=ET150713234951201708&custlinkid=1551677) | 
| INA219: [example](https://nl.aliexpress.com/item/4000330275495.html?spm=a2g0s.9042311.0.0.28c74c4d10PHbJ) |
| N-channel mosfet switchable by 3.3V digital signal: example FQP30N06L |
| inductor, diode, capacitor |

## Implementation

Only the conceptual implementation is explained here, the source files contain the details.
![conceptual implementation](concept.svg)

### Power electronics

- To reliably create the supply voltage for the pressure sensor (~24V), a feedback mechanism is used:
-- the INA219 sends the actual measured voltage over I2C to the wemos
-- in the Wemos a feedback loop (PI controller) adjusts the duty ratio of the mosfet, this guarantees a constant voltage over temperature etc.
- Pin D8 of the WEMOS has to be used to generate the PWM signal for the dc-converter, when going to deepsleep this pin is inherently pulled down and the converter stops switching safely

### Current measurement
Typical breakout boards using the INA219, come together with a 0.1 Ohm shunt resistor.
Only 4-20 mA is produced by the sensor, this means the voltage measured over the shunt would only be 0.4-2 mV which is basically noise ... .
According the datasheet, the range of the INA219 can be configured from 40 to 320mV, hence a resistor in the range of 2 to 15 Ohm would be acceptable. Here, a resistor of 10Ohm is used.
![resistor desoldering](pictures/INA219.png)

## Calibration

Now the hardware is setup, a calibration was done to correlate the measured shunt voltage / current with the pressure.
I've inserted the pressure sensor in water and came to this table:

| Level (meter)       | Shunt voltage mV     | shunt current mA     |
| :------------- | :----------: | -----------: |
|  0 (air) | 51   | 5.1    |
| 1   | 85 | 8.5 | 
| 1.5   | 99 | 9.9 | |

Luckily this leads to a very linear trend :)
![trend line](pictures/trendline.png)


## Pictures

Connections around the well were embedded in a two-component silicone gel:
![connections](pictures/potting.jpg)

PCB board used (still with additional options for prototyping)

Looks of the sensor:
![pressure sensor](pictures/sensor.jpg)


## Home assistant integration

a few mqtt topics were defined to read out and visualize the data:
```
  - platform: mqtt
    name: "Water level depth"
    state_topic: /sensor/WEMOSD1-14269530/data/waterdepth
    expire_after: 864000
    unit_of_measurement: "m"
    payload_available: "Online"
    payload_not_available: "water level sensor offline"
```

## Open issues

- clean up Kicad files (too many items added for debugging purposes)
-- remove incorrect R_pulldown1
