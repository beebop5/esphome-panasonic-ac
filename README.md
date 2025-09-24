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

## Testing

Use the provided test configuration to verify your setup:

```bash
# Activate virtual environment
source venv/bin/activate

# Compile test configuration
esphome compile test_config.yaml

# Upload to device (replace with your device's IP)
esphome upload test_config.yaml
```

## Development

### Project Structure

```
components/panasonic_ac/
├── __init__.py           # Component initialization
├── climate.py            # ESPHome climate integration
├── esppac.cpp            # Core AC communication logic
├── esppac.h              # Header file
├── panasonic_ac_base.cpp # Base communication class
├── panasonic_ac_base.h   # Base class header
└── manifest.json         # Component metadata
```

### Building

1. Set up development environment:
   ```bash
   ./activate_venv.sh
   ```

2. Compile test configuration:
   ```bash
   esphome compile test_config.yaml
   ```

3. Run tests:
   ```bash
   ./test_component.sh
   ```

## Troubleshooting

### Common Issues

1. **No communication with AC unit**
   - Check wiring connections
   - Verify UART settings (9600 baud, even parity)
   - Ensure proper voltage levels

2. **Component not found**
   - Verify external_components configuration
   - Check ESPHome version (1.15.0+ required)

3. **Swing controls not working**
   - Ensure template select components are properly configured
   - Check that IDs match between select components and climate configuration

### Debug Mode

Enable debug logging to troubleshoot communication issues:

```yaml
logger:
  level: DEBUG
```

## Contributing

Contributions are welcome! Please feel free to submit issues and pull requests.

### Development Setup

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Based on reverse engineering of Panasonic AC communication protocols
- Built for the ESPHome ecosystem
- Community contributions and testing

## Support

- **Issues**: [GitHub Issues](https://github.com/beebop5/esphome-panasonic-ac/issues)
- **Documentation**: [Component Documentation](https://github.com/beebop5/esphome-panasonic-ac)
- **ESPHome**: [ESPHome Documentation](https://esphome.io/)

## Version History

- **v0.9**: Initial release with full climate control and swing support
  - Clean codebase without superfluous internal component creation
  - External select/switch component integration
  - Comprehensive configuration examples
