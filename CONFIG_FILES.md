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

## Development Workflow:

1. **Make changes** to `components/panasonic_ac/` files
2. **Test locally** with `test_config_local.yaml`
3. **Commit and push** to GitHub
4. **Verify** with `test_config.yaml` (pulls from GitHub)
5. **Deploy** to actual device

## Quick Test:
```bash
# Activate venv (Windows PowerShell)
& venv\Scripts\Activate.ps1

# Test local version
esphome compile test_config_local.yaml

# Test GitHub version
esphome compile test_config.yaml
```

