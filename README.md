# ESPHome Panasonic AC Component (WLAN + CNT)

A custom ESPHome component for controlling Panasonic air conditioning units via UART communication through either:
- **WLAN interface (CN-WLAN port)** (DNSK-P11 compatible)
- **CNT interface (CN-CNT port)** (CZ-TACG1 compatible)

This provides a local-control alternative to Panasonic adapters, without Comfort Cloud.

## Features

* 🌡️ **Climate Control**: Full temperature control with heating, cooling, dry, and fan modes
* 🌪️ **Fan Control**: Automatic and manual fan speed control (1-5 levels)
* 🎛️ **Swing Control**: Independent horizontal and vertical air swing positioning
* ✨ **NanoeX Support**: Control for Panasonic's nanoeX air purification technology
* 📊 **Temperature Monitoring**: Built-in outside temperature sensor support
* 🔄 **Home Assistant Integration**: Seamless integration with Home Assistant's climate platform
* ⚡ **Local Control**: Instant control without cloud delays

## Supported Hardware

### CN-WLAN (DNSK-P11 compatible)

This component works with Panasonic AC units that have a WLAN interface (CN-WLAN port). It serves as a drop-in replacement for the Panasonic DNSK-P11 wireless LAN adapter. Tested as a drop in replacement for part number ACXA73-28520 on:
- CS-RU12YKA
- CS-RU18YKA
- CS-RU24YKA

### CN-CNT (CZ-TACG1 compatible)

This component also supports the **CZ-TACG1 protocol via CN-CNT** (often called “CNT” in configs).



### Requirements

* ESP32 (recommended) or ESP8266
* 5V to 3.3V bi-directional logic level converter (minimum 2 channels)
* Jumper wires for connections
* ESPHome 2025.12.1 or newer (v1.0.0+ requires 2025.12.1+)
* Home Assistant 2024.1.0 or newer

**Note**: Select the protocol with `type:` in the `climate:` config. Default is `wlan`.

## Breaking Changes in v1.0.0

⚠️ **Version 1.0.0 requires ESPHome 2025.12.1 or newer** due to API changes in the Climate component.

### What Changed:
- **ESPHome Requirement**: Minimum version increased from 2022.5.0 to **2025.12.1+**
- **Fan Mode & Preset Status**: The climate entity's `custom_fan_mode` and `custom_preset` attributes no longer update automatically from AC telemetry (ESPHome API limitation)

### Migration Guide:
To restore fan mode and preset status reporting, add optional text sensors to your configuration:

```yaml
climate:
  - platform: panasonic_ac
    name: "AC"
    # ... other config ...
    fan_mode_status:
      name: "AC Fan Mode Status"
    preset_status:
      name: "AC Preset Status"
```

These sensors will report the current fan mode (Automatic, 1-5) and preset (Normal, Powerful, Quiet) from AC telemetry.

## Installation

### Software Setup

Add the external component to your ESPHome configuration:

**For stable releases (recommended):**
```yaml
external_components:
  source:
    type: git
    url: https://github.com/beebop5/esphome-panasonic-ac
    ref: v1.0.0
  components: [panasonic_ac]
```

**For latest development version:**
```yaml
external_components:
  source:
    type: git
    url: https://github.com/beebop5/esphome-panasonic-ac
    ref: master
  components: [panasonic_ac]
```

### Hardware Wiring

Connect your ESP to the AC unit's CN-WLAN port using a logic level converter:

```
ESP32/ESP8266 (3.3V)    Logic Converter    Panasonic AC CN-WLAN (5V)
GPIO17 (TX)         →   TX             →   RX
GPIO16 (RX)         →   RX             →   TX
GND                 →   GND            →   GND
3.3V                →   LV             
5V                  →   HV             →   VCC
```

### Safety Warnings

* ⚠️ **Always disconnect mains power** before opening your AC unit
* ⚠️ **Never connect ESP directly to AC** - the AC uses 5V while ESP uses 3.3V
* ⚠️ **Use a proper logic level converter** to prevent damage to your ESP
* ⚠️ **Installation at your own risk** - no responsibility for damage to equipment

## Configuration

### Complete Example Configuration (CN-WLAN)

```yaml
substitutions:
  device_name: bedroom-ac
  device_name_short: bedroom
  wifi_ssid: "YourWiFiSSID"
  wifi_pass: "YourWiFiPassword"
  api_key: "YourAPIKey"
  ota_pw: "YourOTAPassword"

esphome:
  name: $device_name
  comment: $device_name

esp32:
  board: esp32dev
  framework: 
    type: arduino
    
wifi:
  networks:
  - ssid: $wifi_ssid
    password: $wifi_pass

api:
  encryption:
    key: $api_key
  
ota:
  - platform: esphome
    password: $ota_pw

logger:
  level: DEBUG

time:
  - platform: sntp
    id: my_time
    servers:
    - 0.pool.ntp.org
    - 1.pool.ntp.org

# UART connection to AC unit
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  parity: EVEN
  rx_buffer_size: 1024

external_components:
  source:
    type: git
    url: https://github.com/beebop5/esphome-panasonic-ac
    ref: v1.0.0
  components: [panasonic_ac]

# Select controls for swing positioning
select:
  - platform: template
    name: "Panasonic AC Horizontal Swing"
    id: panasonic_ac_hswing
    options:
      - "Left"
      - "Centre Left"
      - "Centre"
      - "Centre Right"
      - "Right"
    initial_option: "Centre"
    optimistic: true

  - platform: template
    name: "Panasonic AC Vertical Swing"
    id: panasonic_ac_vswing
    options:
      - "Up"
      - "Mid Up"
      - "Mid"
      - "Mid Down"
      - "Down"
    initial_option: "Mid"
    optimistic: true

# Optional switches for AC features
switch:
  - platform: restart
    name: "Restart"
  - platform: template
    name: "Panasonic AC NanoeX"
    id: panasonic_ac_nanoex
    restore_mode: RESTORE_DEFAULT_OFF
    optimistic: true
  - platform: template
    name: "Panasonic AC Powerful"
    id: panasonic_ac_powerful
    restore_mode: RESTORE_DEFAULT_OFF
    optimistic: true
  - platform: template
    name: "Panasonic AC Quiet"
    id: panasonic_ac_quiet
    restore_mode: RESTORE_DEFAULT_OFF
    optimistic: true

# Climate component with all features
climate:
  - platform: panasonic_ac
    type: wlan
    name: $device_name_short
    outside_temperature:
      name: "Outside Temperature"
    horizontal_swing_select_id: panasonic_ac_hswing
    vertical_swing_select_id: panasonic_ac_vswing
    nanoex_switch_id: panasonic_ac_nanoex
    powerful_switch_id: panasonic_ac_powerful
    quiet_switch_id: panasonic_ac_quiet
    fan_mode_status:
      name: "AC Fan Mode Status"
    preset_status:
      name: "AC Preset Status"

### Minimal Example (CN-CNT / CZ-TACG1)

For CN-CNT, set `type: cnt`. (Select/switch IDs are optional, but recommended.)

```yaml
climate:
  - platform: panasonic_ac
    type: cnt
    name: "Office AC"
    # Optional wiring-dependent entities:
    # vertical_swing_select_id: my_vswing
    # horizontal_swing_select_id: my_hswing
    # nanoex_switch_id: my_nanoex
    # powerful_switch_id: my_powerful
    # quiet_switch_id: my_quiet
```

## Local Development / Testing

ESPHome 2026.x requires **Python 3.11+**. To run the same compile locally:

```bash
cd beebop-panasonic-ac
/usr/local/bin/python3.11 -m venv .venv
. .venv/bin/activate
python -m pip install -U pip
python -m pip install esphome==2026.1.0
./scripts/compile_all.sh
```
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
| `powerful_switch_id` | ID | No | ID of template switch for Powerful mode control |
| `quiet_switch_id` | ID | No | ID of template switch for Quiet mode control |
| `fan_mode_status` | text_sensor | No | Text sensor to report current fan mode from AC telemetry |
| `preset_status` | text_sensor | No | Text sensor to report current preset from AC telemetry |

### UART Configuration

| Option | Value | Description |
|--------|-------|-------------|
| `tx_pin` | GPIO pin | Transmit pin (typically GPIO17) |
| `rx_pin` | GPIO pin | Receive pin (typically GPIO16) |
| `baud_rate` | 9600 | Communication speed |
| `parity` | EVEN | Required for Panasonic AC communication |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Based on reverse engineering of Panasonic AC communication protocols
- Built for the ESPHome ecosystem
- Community contributions and testing
- Related work: [DomiStyle/esphome-panasonic-ac](https://github.com/DomiStyle/esphome-panasonic-ac) - A popular ESPHome Panasonic AC component for CN-CNT port interface

## Support

- **Issues**: [GitHub Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- **Documentation**: [Component Documentation](https://github.com/beebop5/esphome-panasonic-ac)
- **ESPHome**: [ESPHome Documentation](https://esphome.io/)
