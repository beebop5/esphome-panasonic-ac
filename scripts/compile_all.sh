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

echo "Using esphome: $(esphome version | head -n 1)"

echo "==> Validate WLAN config (test_config_local.yaml)"
esphome config test_config_local.yaml

echo "==> Validate CNT config (test_config_cnt_local.yaml)"
esphome config test_config_cnt_local.yaml

echo ""
echo "✅ All configs validated successfully!"
echo ""
echo "Note: Full compilation requires ESP-IDF toolchain setup."
echo "For quick iteration, 'esphome config' validates YAML and component code."
echo "To compile fully, use 'esphome compile' (requires PlatformIO toolchain in PATH)."

