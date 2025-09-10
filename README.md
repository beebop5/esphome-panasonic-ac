# ESPHome Panasonic AC Component

A modern, open-source ESPHome component for controlling Panasonic air conditioners via the WLAN interface. This component provides a drop-in replacement for the Panasonic DNSK-P11 WiFi module, enabling local control without cloud dependencies.

## 🚀 Features

- **Local Control**: Complete local control via Home Assistant, MQTT, or direct ESPHome integration
- **Real-time Communication**: Instant control without the delays of cloud-based solutions
- **Live Status Updates**: Receive real-time temperature, power consumption, and operational status
- **UART Interface**: Direct communication with the AC unit via the CN-WLAN port
- **WLAN-Only Focus**: Streamlined implementation focused exclusively on the WLAN interface
- **Modern Architecture**: Built on ESPHome's latest component framework

## 🏠 Supported Hardware

This component works with Panasonic AC units that have a **CN-WLAN port** and is designed as a replacement for the Panasonic DNSK-P11 WiFi module.

### Compatible AC Units
- Panasonic AC units with CN-WLAN connector (typically newer models)
- Units that previously used or can use the DNSK-P11 WiFi module

### ESP Hardware Requirements
- **ESP32** (recommended) - Multiple hardware serial ports
- **ESP8266** (supported) - Single hardware serial port

## 📋 Requirements

### Hardware
- ESP32 or ESP8266 development board
- 5V to 3.3V bi-directional logic level converter (minimum 2 channels)
- Female-to-female jumper cables
- Soldering iron and basic soldering supplies
- Heat shrink tubing
- Appropriate connectors for your AC's CN-WLAN port

### Software
- ESPHome 2022.5.0 or newer
- Home Assistant 2021.8.0 or newer

## ⚠️ Important Safety Notes

> **⚠️ CRITICAL SAFETY WARNINGS**
> 
> - **ALWAYS disconnect mains power before opening your AC unit**
> - **Mains contacts are exposed and can be touched by accident**
> - **NEVER connect ESP32/ESP8266 directly to the AC - use a logic level converter**
> - **The AC uses 5V while ESPs use 3.3V - direct connection will damage your ESP**
> - **Installation is at your own risk - no responsibility is taken for any damage**

## 🛠️ Installation

### Software Setup

1. **Clone or download this repository**
   ```bash
   git clone https://github.com/beebop5/esphome-panasonic-ac.git
   ```

2. **Copy the example configuration**
   ```bash
   cp panasonic_ac_wlan.yaml.example ac.yaml
   ```

3. **Configure your AC settings**
   - Edit `ac.yaml` to match your ESP hardware pins
   - Configure WiFi settings
   - Adjust AC-specific features as needed

4. **Test the configuration**
   ```bash
   esphome ac.yaml run
   ```

5. **Verify communication**
   - Look for handshake messages in the logs
   - If you see successful communication, proceed to hardware installation

### Hardware Installation

For detailed hardware installation instructions, see: [WLAN Installation Guide](README.WLAN_INSTALLATION.md)

## ⚙️ Configuration

### Basic Configuration

```yaml
external_components:
  source: github://beebop5/esphome-panasonic-ac
  components: [panasonic_ac]

climate:
  - platform: panasonic_ac
    type: wlan
    name: "Panasonic AC"
    
    # Optional features
    horizontal_swing_select:
      name: "AC Horizontal Swing"
    vertical_swing_select:
      name: "AC Vertical Swing"
    outside_temperature:
      name: "AC Outside Temperature"
    nanoex_switch:
      name: "AC NanoeX"
```

### Available Features

Configure only the features supported by your AC unit:

| Feature | Description | Notes |
|---------|-------------|-------|
| `horizontal_swing_select` | Horizontal air swing control | Auto, Left, Center, Right positions |
| `vertical_swing_select` | Vertical air swing control | Auto, Up, Center, Down positions |
| `outside_temperature` | External temperature sensor | Reports outdoor temperature |
| `nanoex_switch` | NanoeX air purification | Available on compatible models |

> **⚠️ Feature Compatibility Warning**
> 
> Only enable features that are supported by your specific AC model. Check your AC's remote control or manual to determine supported features. Enabling unsupported features may cause undefined behavior.

## 🔧 Advanced Configuration

### UART Settings
```yaml
uart:
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600
  parity: EVEN
```

### Logging
```yaml
logger:
  level: DEBUG  # Use INFO for production
```

## 📊 Monitoring and Diagnostics

### Log Messages
- **Handshake messages**: Indicate successful AC communication
- **Temperature updates**: Real-time temperature reporting
- **Command acknowledgments**: Confirmation of control commands

### Troubleshooting
1. **No handshake messages**: Check UART connections and pin configuration
2. **Intermittent communication**: Verify power supply and connection stability
3. **Feature not working**: Ensure the feature is supported by your AC model

## 🔄 Upgrading

For upgrade instructions from previous versions, see: [Upgrade Guide](README.UPGRADING.md)

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

### Development
- Fork the repository
- Create a feature branch
- Make your changes
- Test thoroughly
- Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Based on the original ESPHome Panasonic AC component
- Community contributions and testing
- ESPHome team for the excellent framework

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- **Discussions**: [GitHub Discussions](https://github.com/beebop5/esphome-panasonic-ac/discussions)
- **ESPHome Community**: [ESPHome Discord](https://discord.gg/KhAMKrd)

---

**Made with ❤️ for the Home Assistant community**