# ESPHome Panasonic AC Component

An ESPHome component for controlling Panasonic air conditioners via the WLAN interface. Drop-in replacement for the Panasonic DNSK-P11 WiFi module.

## Features

- Local control via Home Assistant or MQTT
- Real-time temperature and status updates
- WLAN interface support (CN-WLAN port)
- No cloud dependencies

## Requirements

- ESP32 or ESP8266
- 5V to 3.3V logic level converter
- ESPHome 2022.5.0+
- Home Assistant 2021.8.0+

## Installation

1. **Copy the example config:**
   ```bash
   cp panasonic_ac_wlan.yaml.example ac.yaml
   ```

2. **Update the config:**
   ```yaml
   external_components:
     source: github://beebop5/esphome-panasonic-ac
     components: [panasonic_ac]
   
   climate:
     - platform: panasonic_ac
       type: wlan
       name: "Panasonic AC"
   ```

3. **Flash and test:**
   ```bash
   esphome ac.yaml run
   ```

4. **Install hardware:** See [WLAN Installation Guide](README.WLAN_INSTALLATION.md)

## Configuration

Basic configuration with optional features:

```yaml
climate:
  - platform: panasonic_ac
    type: wlan
    name: "Panasonic AC"
    
    # Optional features (enable only if supported by your AC)
    horizontal_swing_select:
      name: "AC Horizontal Swing"
    vertical_swing_select:
      name: "AC Vertical Swing"
    outside_temperature:
      name: "AC Outside Temperature"
    nanoex_switch:
      name: "AC NanoeX"
```

## Safety Warning

⚠️ **Always disconnect mains power before opening your AC unit. Use a logic level converter - never connect ESP directly to AC (5V vs 3.3V).**

## License

MIT License - see [LICENSE](LICENSE) for details.

## Support

- [Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- [Discussions](https://github.com/beebop5/esphome-panasonic-ac/discussions)