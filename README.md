# ESPHome Panasonic AC WLAN Component

An open source alternative for Panasonic air conditioning wifi adapters that works locally without the Comfort Cloud.

## Overview

A custom ESPHome component for controlling Panasonic air conditioning units via the WLAN interface (CN-WLAN port). This component serves as a replacement for the Panasonic DNSK-P11 wireless LAN adapter (tested with part number ACXA73-28520) and provides full climate control functionality through the WLAN port.

## Features

* Control your AC locally via Home Assistant, MQTT or directly
* Instantly control the AC without any delay like in the Comfort Cloud app
* Receive live reports and state from the AC
* Uses the UART interface on the AC instead of the IR interface
* Provides a drop-in replacement for the Panasonic DNSK-P11 wifi module via WLAN interface
* Full temperature control with heating, cooling, and dry modes
* Automatic and manual fan speed control (1-5 levels)
* Independent horizontal and vertical air swing positioning
* Control for Panasonic's nanoeX air purification technology
* Built-in outside temperature sensor support

## Supported hardware

This component works with Panasonic AC units that have a WLAN interface (CN-WLAN port) and use UART communication. It has been tested with models that support the CZ-TACG1 protocol through the WLAN port.

**Note**: This component is specifically designed for the WLAN interface (CN-WLAN port) and serves as a replacement for the DNSK-P11 adapter. For the CN-CNT port interface, please refer to the [DomiStyle/esphome-panasonic-ac](https://github.com/DomiStyle/esphome-panasonic-ac) component.

Works on the ESP8266 but ESP32 is preferred for the multiple hardware serial ports.

## Requirements

* ESP32 (or ESP8266) (supported by ESPHome)
* 5V to 3.3V bi-directional Logic Converter (minimum 2 channels, available as pre-soldered prototyping boards)
* Female-Female Jumper cables
* Soldering iron
* Wires to solder from Logic converter to ESP
* Heat shrink
* ESPHome 2022.5.0 or newer
* Home Assistant 2021.8.0 or newer

## Notes

* **Make sure to disconnect mains power before opening your AC, the mains contacts are exposed and can be touched by accident!**
* **Do not connect your ESP32/ESP8266 directly to the AC, the AC uses 5V while the ESPs use 3.3V!**
* **While installation is fairly straightforward I do not take any responsibility for any damage done to you or your AC during installation**

## Software installation

This software installation guide assumes some familiarity with ESPHome.

* Add this to your ESPHome configuration:

```yaml
external_components:
  source: github://beebop5/esphome-panasonic-ac
  components: [panasonic_ac]
```

* Copy the configuration from `panasonic_ac.yaml.example` in the root folder
* Adjust the configuration to your needs
* Connect your ESP
* Run `esphome your_config.yaml run` and choose your serial port (or do this via the Home Assistant UI)
* If you see the handshake messages being sent in the log you are good to go
* Disconnect the ESP and continue with hardware installation

## Setting supported features

Since Panasonic ACs support different features you can comment out the lines in your configuration:

```yaml
# Enable as needed
# outside_temperature_sensor:
#   name: "Panasonic AC Outside Temperature"
# vertical_swing_select:
#   name: "Panasonic AC Vertical Swing"
# horizontal_swing_select:
#   name: "Panasonic AC Horizontal Swing"
# nanoex_switch:
#   name: "Panasonic AC NanoeX"
```

In order to find out which features are supported by your AC, check the remote that came with it.

**Enabling unsupported features can lead to undefined behavior and may damage your AC. Make sure to check your remote or manual first.**

## Hardware installation

### Wiring Diagram

Connect your ESP32/ESP8266 to the AC unit's WLAN interface (CN-WLAN port) using a logic level converter:

```
ESP32/ESP8266    Logic Converter    Panasonic AC (CN-WLAN)
GPIO17 (TX)      →    TX            RX
GPIO16 (RX)      →    RX            TX
GND              →    GND           GND
3.3V/5V          →    VCC           VCC (check AC unit requirements)
```

**Important**: This component connects to the CN-WLAN port (where the DNSK-P11 adapter normally connects), not the CN-CNT port.

**Warning**: Ensure proper voltage levels and isolation if needed. Some AC units may require level shifters or optocouplers.

## Configuration Example

```yaml
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 2400
  data_bits: 8
  parity: even
  stop_bits: 1

panasonic_ac:
  id: ac1
  uart_id: uart1

climate:
  - platform: panasonic_ac
    name: "Panasonic AC"
    panasonic_ac_id: ac1
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Based on reverse engineering of Panasonic AC communication protocols
- Developed for the ESPHome ecosystem
- Community contributions and testing
- Related work: [DomiStyle/esphome-panasonic-ac](https://github.com/DomiStyle/esphome-panasonic-ac) - An ESPHome Panasonic AC component that provides similar functionality for different AC models

## Support

- **Issues**: [GitHub Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- **Documentation**: [Component Documentation](https://github.com/beebop5/esphome-panasonic-ac)
- **ESPHome**: [ESPHome Documentation](https://esphome.io/)

## Appendix: Communication Protocol

This section documents the key-value pairs used in the UART communication protocol with Panasonic AC units.

### Protocol Overview

The communication protocol uses a custom UART-based protocol with the following characteristics:
- Baud rate: 2400
- Data bits: 8
- Parity: Even
- Stop bits: 1
- Header: 0x5A

### Key-Value Pairs

| Key | Value | Description |
|-----|-------|-------------|
| `0x14` | `0x31` | Power OFF |
| `0x14` | `0x30` | Power ON |
| `0x18` | `0x30` | Auto mode |
| `0x18` | `0x31` | Cool mode |
| `0x18` | `0x32` | Heat mode |
| `0x18` | `0x33` | Dry mode |
| `0x18` | `0x34` | Fan only mode |
| `0x22` | `0x10-0x1E` | Target temperature (16-30°C) |
| `0x26` | `0x30-0x34` | Fan speed (Auto, 1, 2, 3, 4, 5) |
| `0x30` | `0x30` | Swing OFF |
| `0x30` | `0x31` | Swing ON |
| `0x34` | `0x30-0x34` | Horizontal swing position |
| `0x38` | `0x30-0x34` | Vertical swing position |
| `0x42` | `0x30` | Normal preset |
| `0x42` | `0x31` | Powerful preset |
| `0x42` | `0x32` | Quiet preset |
| `0x50` | `0x30` | NanoeX OFF |
| `0x50` | `0x31` | NanoeX ON |

### Packet Structure

Each packet follows this structure:
1. Header (0x5A)
2. Packet counter
3. Command type
4. Data length
5. Data payload
6. Checksum

The checksum is calculated by summing all bytes in the packet (excluding the checksum byte itself).