#!/bin/bash

echo "🧪 Testing ESPHome Panasonic AC Component"
echo "========================================"

# Check if ESPHome is installed
if ! command -v esphome &> /dev/null; then
    echo "❌ ESPHome not found. Installing..."
    pip install esphome
fi

echo "📋 Testing component structure..."
# Check required files
required_files=(
    "components/panasonic_ac/manifest.json"
    "components/panasonic_ac/climate.py"
    "components/panasonic_ac/panasonic_ac_base.h"
    "components/panasonic_ac/panasonic_ac_base.cpp"
    "components/panasonic_ac/esppac_wlan.h"
    "components/panasonic_ac/esppac_wlan.cpp"
)

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "✅ $file exists"
    else
        echo "❌ $file missing"
        exit 1
    fi
done

# Check removed files don't exist
removed_files=(
    "components/panasonic_ac/panasonic_ac_select.h"
    "components/panasonic_ac/panasonic_ac_switch.h"
    "components/panasonic_ac/panasonic_ac_wlan_commands.h"
)

for file in "${removed_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo "✅ $file correctly removed"
    else
        echo "❌ $file should be removed"
        exit 1
    fi
done

echo ""
echo "🔨 Testing compilation..."

# Test ESP32 compilation
echo "Testing ESP32 compilation..."
if esphome compile test_config.yaml --board esp32dev; then
    echo "✅ ESP32 compilation successful"
else
    echo "❌ ESP32 compilation failed"
    exit 1
fi

# Test ESP8266 compilation
echo "Testing ESP8266 compilation..."
if esphome compile test_config.yaml --board nodemcuv2; then
    echo "✅ ESP8266 compilation successful"
else
    echo "❌ ESP8266 compilation failed"
    exit 1
fi

echo ""
echo "🎉 All tests passed! Component is ready."
