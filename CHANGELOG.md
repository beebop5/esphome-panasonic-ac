# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-01-XX

### Added
- **CN-CNT (CZ-TACG1) protocol support**: Full support for Panasonic AC units with CN-CNT interface
  - New `type: cnt` configuration option for CZ-TACG1 compatible units
  - Complete protocol implementation with polling, command handling, and state synchronization
  - Support for all climate modes (Off, Heat, Cool, Dry, Fan Only, Auto)
  - Fan speed control (Automatic, 1-5)
  - Horizontal and vertical swing control with "Auto" and "Swing" modes
  - NanoeX switch support
  - Outside temperature sensor support
  - Optional current temperature sensor for units that don't report temperature
- **Enhanced swing mode support for CNT**: 
  - Vertical swing supports "Up", "Mid Up", "Mid", "Mid Down", "Down", "Swing", and "Auto"
  - Horizontal swing supports "Left", "Centre Left", "Centre", "Centre Right", "Right", and "Auto"
- **Local testing infrastructure**:
  - `scripts/compile_all.sh` - Quick validation script for both WLAN and CNT configs
  - `scripts/compile_all_full.sh` - Full compilation script for both protocols
  - `test_config_cnt_local.yaml` - Local test configuration for CNT interface
  - `CONFIG_FILES.md` - Documentation for test configuration files

### Changed
- **Component architecture**: Refactored to support multiple protocol types via `cv.typed_schema`
  - `climate.py` now uses typed schema to differentiate between WLAN and CNT types
  - Base class (`PanasonicACBase`) provides common functionality
  - Protocol-specific implementations (`PanasonicAC` for WLAN, `PanasonicACCNT` for CNT)
- **Swing option labels**: Standardized swing mode labels to capitalized format (e.g., "Auto", "Swing", "Up") for consistency between WLAN and CNT
- **Logging improvements**: Enhanced CNT logging to match WLAN verbosity and detail level
- **Text sensor updates**: Guarded text sensor status updates with `USE_TEXT_SENSOR` preprocessor directives for better compatibility

### Fixed
- **Swing state filtering**: Filter out "unsupported" and "Unknown" swing states before publishing to select components to prevent invalid option errors
- **Code generation**: Fixed codegen to correctly instantiate `PanasonicACCNT` when `type: cnt` is specified
- **StringRef compatibility**: All `strcmp` calls now use `.c_str()` on `esphome::StringRef` objects for ESPHome 2025.12.1+ compatibility

### Documentation
- Updated README.md with CNT configuration examples and protocol selection guide
- Added comprehensive configuration examples for both WLAN and CNT interfaces
- Documented swing mode options for both protocols
- Added local testing setup instructions

## [1.0.0] - 2024-XX-XX

### Added
- Initial release with WLAN (DNSK-P11) protocol support
- Full climate control (temperature, mode, fan speed)
- Horizontal and vertical swing control
- NanoeX, Powerful, and Quiet mode switches
- Outside temperature sensor support
- Optional text sensors for fan mode and preset status reporting

### Breaking Changes
- **ESPHome Requirement**: Minimum version increased from 2022.5.0 to **2025.12.1+**
- **Fan Mode & Preset Status**: The climate entity's `custom_fan_mode` and `custom_preset` attributes no longer update automatically from AC telemetry (ESPHome API limitation)

### Migration Guide
To restore fan mode and preset status reporting, add optional text sensors to your configuration:
```yaml
climate:
  - platform: panasonic_ac
    name: "AC"
    fan_mode_status:
      name: "AC Fan Mode Status"
    preset_status:
      name: "AC Preset Status"
```
