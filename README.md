# ArduZombieOS

* read up to 16 push buttons
* blink up to 16 LEDs (push button LEDs)
* up to 16 outputs
* connect to ethernet (DHCP)
* connect to MQTT server
* send mqtt message on output state change or state query
* subscribe to mqtt and output changes
* output state can be 0 = off, 1 = on, >1 = seconds till stop

Uses [FreeRTOS](https://github.com/feilipu/Arduino_FreeRTOS_Library) port for Arduino.

### Requirements
- Arduino MEGA 2560 board with W5100 ethernet shield
- Arduino AVR Boards version 1.8.6
- FreeRTOS version 10.3.0-9

### Operation mode

#### On button short press:
  - toggle output
  - send mqtt message `house/:id/light/state/:output` with payload `0` or `1`

##### On MQTT message:
  - `house/:id/light/:output` with payload `0` or `1`: set output on or off. If `:output` is `0`, turn off all outputs. If payload is a number > 1, set output on for that many seconds.
  - `house/:id` send status for all outputs that are on
  - `house/:id/reset` reset the device
  - `house/:id/set` update setting, when payload
    - `?` - reset settings to default
    - `iX` - set id to `X` eg: `i1`
    - `hHOSTNAME` - set hostname to `HOSTNAME` eg: `hArdu`
    - `lXY` - associate output `X` with button `Y` eg: `l0F`, X, Y is [0-F]
    - `bX`, X=0..4 - set blink mode


### Pinout

#### Inputs (Buttons) from 1 to 16 (PORTA, PORTC)

- multiplexed with button LED:
  - LED+ = 5V
  - LED- = pin
  - ButtonNO1 = pin
  - ButtonNO2 = GND

- Arduino PINS: 22 - 29, 37 - 30

#### Outputs (Relays) from 1 to 16 (ADC/PORTK, PORTL)

- Arduino PINS: A8 - A15, 42 - 49

### Arduino MEGA 2560 Pinout

![Arduino MEGA 2560 Pinout](/assets/arduino-mega-pinout.png)
