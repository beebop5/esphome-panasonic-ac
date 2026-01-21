# Configuration Files

## test_config_local.yaml
**Use for:** Local development and testing of component changes

**Source:** `components/` directory (local files)

**Purpose:**
- Test changes to the Panasonic AC component before committing
- Quick iteration during development
- Verify compilation with local modifications

**Compile:**
```bash
esphome compile test_config_local.yaml
```

## test_config.yaml
**Use for:** Testing with the published GitHub version

**Source:** `https://github.com/beebop5/esphome-panasonic-ac` (remote repository)

**Purpose:**
- Test the published version from GitHub
- Verify that pushed changes work correctly
- Test like an end user would experience it

**Compile:**
```bash
esphome compile test_config.yaml
```

## Both Configurations Include:
✅ All critical bug fixes (0x3A header support, query response size fix, etc.)
✅ Powerful and Quiet switches
✅ NanoeX switch
✅ Horizontal and Vertical swing selects
✅ Outside temperature sensor

## CNT Configuration

- For CN-CNT / CZ-TACG1, use `test_config_cnt_local.yaml` and set `type: cnt` under the `climate:` block.

## Development Workflow:

1. **Make changes** to `components/panasonic_ac/` files
2. **Test locally** with `test_config_local.yaml`
3. **Commit and push** to GitHub
4. **Verify** with `test_config.yaml` (pulls from GitHub)
5. **Deploy** to actual device

## Local Build Setup (recommended)

ESPHome 2026.x requires **Python 3.11+**.

```bash
cd beebop-panasonic-ac
/usr/local/bin/python3.11 -m venv .venv
. .venv/bin/activate
python -m pip install -U pip
python -m pip install esphome==2026.1.0
```

## Validate Both WLAN + CNT (local components)

**Recommended for local testing:** Use `esphome config` to validate YAML and component code without full compilation:

```bash
cd beebop-panasonic-ac
. .venv/bin/activate
./scripts/compile_all.sh
```

This validates both configs and catches syntax/component integration errors quickly.

## Full Compilation

**Note:** ESPHome 2026.1.0 uses both Arduino and ESP-IDF frameworks by default. Full local compilation requires a complete ESP-IDF environment setup, which can be complex.

**Recommended:** Use Home Assistant's ESPHome add-on for full compiles, as it has the ESP-IDF toolchain properly configured.

If you need full local compiles, use:
```bash
cd beebop-panasonic-ac
. .venv/bin/activate
./scripts/compile_all_full.sh
```

**Note:** This may fail with ESP-IDF linker script errors if the ESP-IDF environment isn't fully set up. The `compile_all.sh` script (using `esphome config`) is sufficient for catching most component issues.

## Quick Test:
```bash
# Activate venv (Windows PowerShell)
& venv\Scripts\Activate.ps1

# Test local version
esphome compile test_config_local.yaml

# Test GitHub version
esphome compile test_config.yaml
```

