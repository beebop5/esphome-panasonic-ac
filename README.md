# ESPHome Panasonic AC Component

A custom ESPHome component for controlling Panasonic air conditioning units via UART communication. This component provides full climate control functionality with support for temperature control, fan speeds, operation modes, and advanced features like swing control and nanoeX air purification.

## Features

- 🌡️ **Climate Control**: Full temperature control with heating, cooling, and dry modes
- 🌪️ **Fan Control**: Automatic and manual fan speed control (1-5 levels)
- 🎛️ **Swing Control**: Independent horizontal and vertical air swing positioning
- ✨ **NanoeX Support**: Control for Panasonic's nanoeX air purification technology
- 📊 **Temperature Monitoring**: Built-in outside temperature sensor support
- 🔄 **Home Assistant Integration**: Seamless integration with Home Assistant's climate platform
- 📱 **ESPHome Dashboard**: Full control via ESPHome's web interface

## Supported Models

This component is designed for Panasonic AC units that use UART communication. It has been tested with models that support the CZ-TACG1 protocol.

## Hardware Requirements

- ESP32 or ESP8266 microcontroller
- UART connection to your Panasonic AC unit
- Appropriate wiring (see wiring section)

## Installation

### Method 1: External Component (Recommended)

Add this to your ESPHome configuration:

```yaml
external_components:
  source: github://beebop5/esphome-panasonic-ac
  components: [panasonic_ac]
```

### Method 2: Local Development

1. Clone this repository
2. Copy the `components/panasonic_ac` folder to your ESPHome custom components directory
3. Use the component in your configuration

## Configuration

### Basic Setup

```yaml
# UART configuration (ESPHome 1.15.0+ required for even parity)
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  parity: EVEN

# External component reference
external_components:
  source: github://beebop5/esphome-panasonic-ac
  components: [panasonic_ac]

# Climate component
climate:
  - platform: panasonic_ac
    name: "Living Room AC"
    outside_temperature:
      name: "AC Outside Temperature"
```

### Advanced Configuration with Swing Control

```yaml
# Template select components for swing control
select:
  - platform: template
    name: "AC Horizontal Swing"
    id: ac_horizontal_swing
    options:
      - "Left"
      - "Centre Left"
      - "Centre"
      - "Centre Right"
      - "Right"
    initial_option: "Centre"
    optimistic: true

  - platform: template
    name: "AC Vertical Swing"
    id: ac_vertical_swing
    options:
      - "Up"
      - "Mid Up"
      - "Mid"
      - "Mid Down"
      - "Down"
    initial_option: "Mid"
    optimistic: true

# Optional nanoeX switch
switch:
  - platform: template
    name: "AC NanoeX"
    id: ac_nanoex
    restore_mode: RESTORE_DEFAULT_OFF
    optimistic: true

climate:
  - platform: panasonic_ac
    name: "Living Room AC"
    outside_temperature:
      name: "AC Outside Temperature"
    # Connect external components
    horizontal_swing_select_id: ac_horizontal_swing
    vertical_swing_select_id: ac_vertical_swing
    nanoex_switch_id: ac_nanoex
```

## Configuration Options

### Climate Component

| Option | Type | Required | Description |
|--------|------|----------|-------------|
| `name` | string | **Yes** | The name for the climate device |
| `outside_temperature` | sensor | No | Temperature sensor for outside temperature monitoring |
| `horizontal_swing_select_id` | ID | No | ID of template select for horizontal swing control |
| `vertical_swing_select_id` | ID | No | ID of template select for vertical swing control |
| `nanoex_switch_id` | ID | No | ID of template switch for nanoeX control |

### UART Configuration

| Option | Value | Description |
|--------|-------|-------------|
| `tx_pin` | GPIO pin | Transmit pin (typically GPIO17) |
| `rx_pin` | GPIO pin | Receive pin (typically GPIO16) |
| `baud_rate` | 9600 | Communication speed |
| `parity` | EVEN | Required for Panasonic AC communication |

## Wiring

Connect your ESP32/ESP8266 to your Panasonic AC unit:

```
ESP32/ESP8266    →    Panasonic AC
GPIO17 (TX)      →    RX
GPIO16 (RX)      →    TX
GND              →    GND
3.3V/5V          →    VCC (check AC unit requirements)
```

**⚠️ Warning**: Ensure proper voltage levels and isolation if needed. Some AC units may require level shifters or optocouplers.


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Based on reverse engineering of Panasonic AC communication protocols
- Built for the ESPHome ecosystem
- Community contributions and testing
- Related work: [DomiStyle/esphome-panasonic-ac](https://github.com/DomiStyle/esphome-panasonic-ac) - A popular ESPHome Panasonic AC component with 313+ stars that provides similar functionality for different AC models

## Support

- **Issues**: [GitHub Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- **Documentation**: [Component Documentation](https://github.com/beebop5/esphome-panasonic-ac)
- **ESPHome**: [ESPHome Documentation](https://esphome.io/)

## Appendix: Communication Protocol

This section documents the key-value pairs used in the UART communication protocol with Panasonic AC units.

### Protocol Overview

- **Baud Rate**: 9600 bps
- **Parity**: Even
- **Data Bits**: 8
- **Stop Bits**: 1
- **Packet Format**: `[Header][Length][Counter][Data][Checksum]`

### Command Key-Value Pairs

#### Power Control
| Key | Value | Description |
|-----|-------|-------------|
| `0x80` | `0x30` | Power ON |
| `0x80` | `0x31` | Power OFF |

#### Operation Mode
| Key | Value | Description |
|-----|-------|-------------|
| `0xB0` | `0x41` | Auto mode |
| `0xB0` | `0x42` | Cool mode |
| `0xB0` | `0x43` | Heat mode |
| `0xB0` | `0x44` | Dry mode |
| `0xB0` | `0x45` | Fan only mode |

#### Temperature Control
| Key | Value | Description |
|-----|-------|-------------|
| `0x31` | `(temp * 2)` | Target temperature (e.g., 0x38 = 28°C) |

#### Fan Speed Control
| Key | Value | Description |
|-----|-------|-------------|
| `0xA0` | `0x32` | Fan speed 1 |
| `0xA0` | `0x33` | Fan speed 2 |
| `0xA0` | `0x34` | Fan speed 3 |
| `0xA0` | `0x35` | Fan speed 4 |
| `0xA0` | `0x36` | Fan speed 5 |
| `0xA0` | `0x41` | Auto fan speed |

#### Preset Modes
| Key | Value | Description |
|-----|-------|-------------|
| `0xB2` | `0x41` | Normal preset |
| `0xB2` | `0x42` | Quiet preset |
| `0xB2` | `0x43` | Powerful preset |

#### Vertical Swing Control
| Key | Value | Description |
|-----|-------|-------------|
| `0xA1` | `0x41` | Up |
| `0xA1` | `0x42` | Down |
| `0xA1` | `0x43` | Centre |
| `0xA1` | `0x44` | Up centre |
| `0xA1` | `0x45` | Down centre |

#### Horizontal Swing Control
| Key | Value | Description |
|-----|-------|-------------|
| `0xA5` | `0x41` | Right |
| `0xA5` | `0x42` | Left |
| `0xA5` | `0x43` | Centre |
| `0xA5` | `0x56` | Right centre |
| `0xA5` | `0x5C` | Left centre |

#### Swing Mode Control
| Key | Value | Description |
|-----|-------|-------------|
| `0x34` | `0x41` | Both swing |
| `0x34` | `0x42` | Off |
| `0x34` | `0x43` | Vertical only |
| `0x34` | `0x44` | Horizontal only |

#### NanoeX Air Purification
| Key | Value | Description |
|-----|-------|-------------|
| `0x33` | `0x42` | NanoeX OFF |
| `0x33` | `0x45` | NanoeX ON |

### Response Packet Types

#### Handshake Packets
| Packet ID | Description |
|-----------|-------------|
| `0x00 0x89` | Handshake step 2 response |
| `0x00 0x8C` | Handshake step 3 response |
| `0x00 0x90` | Handshake step 4 response |
| `0x00 0x91` | Handshake step 5 response |
| `0x00 0x92` | Handshake step 6 response |
| `0x00 0xC1` | Handshake step 7 response |
| `0x01 0xCC` | Handshake step 8 response |
| `0x10 0x80` | Handshake step 9 response |
| `0x10 0x81` | Handshake step 10 response |
| `0x00 0x98` | Handshake step 11 response |
| `0x01 0x80` | Handshake step 12 response |
| `0x10 0x88` | Handshake step 13 response |

#### Status Packets
| Packet ID | Description |
|-----------|-------------|
| `0x01 0x01` | Ping response |
| `0x11 0x01` | Alternative ping response |
| `0x10 0x89` | Status query response |
| `0x10 0x88` | Command acknowledgment |
| `0x10 0x0A` | Status report |
| `0x00 0x20` | Unsolicited status packet |
| `0x00 0x09` | First unsolicited packet with RX counter |

#### Sync Packets
| Value | Description |
|-------|-------------|
| `0x66` | Sync packet (only packet not starting with 0x5A) |

### Status Reading

The component reads AC status from query response packets (`0x10 0x89`) and parses the following fields:

| Offset | Key | Description |
|--------|-----|-------------|
| 14 | `0x80` | Power status (0x30=ON, 0x31=OFF) |
| 15 | `0xB0` | Operation mode |
| 16 | `0x31` | Target temperature |
| 17 | `0xA0` | Fan speed |
| 18 | `0xB2` | Preset mode |
| 19 | `0xA1` | Vertical swing |
| 20 | `0xA5` | Horizontal swing |
| 21 | `0x33` | NanoeX status |

### Temperature Encoding

Temperatures are encoded as `(temperature * 2)`:
- 16°C = 0x20 (32)
- 20°C = 0x28 (40)
- 24°C = 0x30 (48)
- 28°C = 0x38 (56)
- 32°C = 0x40 (64)

