#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if [[ ! -f ".venv/bin/activate" ]]; then
  echo "Missing .venv. Create it first:"
  echo "  /usr/local/bin/python3.11 -m venv .venv"
  exit 1
fi

. .venv/bin/activate

#
# Ensure ESP-IDF's compiler (xtensa-esp32-elf-gcc/g++) is available for CMake.
# PlatformIO toolchain layout is:
#   ~/.platformio/packages/toolchain-xtensa-esp-elf/xtensa-esp-elf/bin
# (and sometimes also nested one level deeper).
#
TOOLCHAIN_BIN_1="${HOME}/.platformio/packages/toolchain-xtensa-esp-elf/xtensa-esp-elf/bin"
TOOLCHAIN_BIN_2="${HOME}/.platformio/packages/toolchain-xtensa-esp-elf/xtensa-esp-elf/xtensa-esp-elf/bin"

if [[ -d "${TOOLCHAIN_BIN_1}" ]]; then
  export PATH="${TOOLCHAIN_BIN_1}:${PATH}"
fi
if [[ -d "${TOOLCHAIN_BIN_2}" ]]; then
  export PATH="${TOOLCHAIN_BIN_2}:${PATH}"
fi

# Set ESP-IDF environment variables
IDF_PATH="${HOME}/.platformio/packages/framework-espidf"
if [[ -d "${IDF_PATH}" ]]; then
  export IDF_PATH
  # ESP-IDF tools might need Python from venv
  export PATH="$(dirname $(which python3)):${PATH}"
fi

echo "Using esphome: $(esphome version | head -n 1)"
echo "IDF_PATH: ${IDF_PATH:-not set}"
echo "Toolchain bin: ${TOOLCHAIN_BIN_1}"
echo "Toolchain bin (alt): ${TOOLCHAIN_BIN_2}"
echo "xtensa-esp32-elf-gcc: $(command -v xtensa-esp32-elf-gcc || echo 'NOT FOUND')"

echo "==> Full compile WLAN config (test_config_local.yaml)"
esphome clean test_config_local.yaml
esphome compile test_config_local.yaml

echo "==> Full compile CNT config (test_config_cnt_local.yaml)"
esphome clean test_config_cnt_local.yaml
esphome compile test_config_cnt_local.yaml

echo "All compiles succeeded."

