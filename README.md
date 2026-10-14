# Wireless Valve Node — ESP8266

Firmware for a remote irrigation valve node based on an ESP8266 WEMOS D1 board. The current application receives 433 MHz RF commands from the Arduino UNO watering controller and pulses the driver for a 6 V DC bi-stable (latching) valve.

> **Current firmware:** RF valve receiver in `src/rf.cpp`. The Wi-Fi/MQTT soil-moisture experiment in `src/main.cpp` is a different application and is excluded from the active build configuration.

## System role

1. Listen for RF commands using the RCSwitch library.
2. Compare the received code with the node's configured valve ID.
3. Generate an OPEN or CLOSE pulse through the valve-driver outputs.
4. Disable the driver output after the pulse.

The node does not currently report valve position or acknowledge commands to the controller. A received RF command is not proof that the valve physically moved.

## Hardware

| Item | Current configuration |
|---|---|
| MCU / board | ESP8266 WEMOS D1 (full-size board for current testing) |
| RF band | 433 MHz |
| RF library | RCSwitch |
| Valve | 6 V DC bi-stable / latching valve |
| Valve ID in source | `VAL_NO = 1` |
| RF receiver signal | D6 / GPIO12 |
| Driver control output 1 | D3 / GPIO0 |
| Driver control output 2 | D1 / GPIO5 |
| Driver power-control output | D5 / GPIO14 |
| Pulse delay in source | 25 ms |
| Wi-Fi | Disabled by the current RF firmware |

The output pins are logic-level control signals. They must connect to a suitable valve-driver circuit, not directly to the valve coil. Confirm electrical details against the actual driver schematic and valve datasheet.

## RF command mapping

| Action | Code for node ID `n` |
|---|---:|
| Open / ON | `2560 + n` |
| Close / OFF | `1280 + n` |

With the current `VAL_NO = 1`, expected codes are:

- Open: `2561`
- Close: `1281`

The controller transmits each command as a 24-bit value twice. See the [Wireless Watering System repository](https://github.com/Radfar/wireless-watering-system) and [`docs/rf-protocol.md`](docs/rf-protocol.md).

## Build environment

This repository uses PlatformIO with the Arduino framework for ESP8266. The active environment is `d1`, targeting the full-size WEMOS D1 / ESP8266 board.

From VS Code with PlatformIO installed:

1. Open this repository folder.
2. Select the `d1` environment.
3. Build the project.
4. Connect the D1 by USB and select the correct serial port.
5. Upload and open the serial monitor if needed.

The active source filter builds `src/rf.cpp` and excludes `src/main.cpp`, because both files currently define their own `setup()` and `loop()`. Do not remove this separation unless the applications are deliberately refactored.

## Current limitations and verification items

- No RF acknowledgement or confirmed valve-position feedback.
- No message authentication or encryption for the fixed RF command codes.
- Valve ID is compiled into firmware; the current source uses ID 1.
- Boot-time code pulses both valve directions. Keep the valve/actuator safe during first power-up tests and verify that this behavior is intentional.
- The driver circuit, coil current, supply behavior, and transient protection must be checked against the actual hardware.
- Validate the 25 ms pulse duration and polarity with the valve datasheet.
- RF range, interference resistance, and command-loss behavior have not been certified by this documentation.
- The MQTT experiment uses plain MQTT in its current source and must not be treated as secure for exposure to untrusted networks.

## Next development stage

Keep the MQTT/soil-moisture application as a separate target or project until the RF valve-node firmware is built and tested reliably. Gateway integration can be added later without coupling basic valve operation to Internet availability.

## License

No license has been specified yet. Add a license before redistributing or commercially reusing the source.
