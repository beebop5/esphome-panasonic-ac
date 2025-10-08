# Switch Operation and Logging Guide

## Powerful and Quiet Switches

### How They Work

The **Powerful** and **Quiet** switches are **mutually exclusive** - turning one on automatically turns the other off.

### Log Messages

When you toggle these switches, you should see these log messages (at DEBUG level):

#### Powerful Switch ON:
```
[D][panasonic_ac.dnskp11]: Turning powerful mode on
```

#### Powerful Switch OFF:
```
[D][panasonic_ac.dnskp11]: Turning powerful mode off
```

#### Quiet Switch ON:
```
[D][panasonic_ac.dnskp11]: Turning quiet mode on
```

#### Quiet Switch OFF:
```
[D][panasonic_ac.dnskp11]: Turning quiet mode off
```

### Troubleshooting: Not Seeing Switch Logs?

If the switches work but you don't see the log messages, check these:

#### 1. Logger Level
Your logger must be set to at least DEBUG level:

```yaml
logger:
  level: DEBUG  # or VERBOSE
```

If set to INFO or higher, DEBUG messages won't appear.

#### 2. AC State
The switches only send commands when the AC component is in the `Ready` state. During initialization, switch toggles are ignored.

Check for this log line first:
```
[I][panasonic_ac]: Panasonic AC component v0.9 initialized
```

#### 3. Switch Triggering
Verify the switches are actually being triggered by checking for these logs:

```
[D][switch:020]: 'Panasonic AC Powerful' Turning ON.
[D][switch:063]: 'Panasonic AC Powerful': Sending state ON
```

If you see these but NOT the "Turning powerful mode on" message, the AC state might not be Ready.

#### 4. Log Filter
If using syslog or filtering, ensure `panasonic_ac` component logs aren't filtered out.

### Verification Steps

1. **Check Logger Level:**
   ```yaml
   logger:
     level: VERBOSE
   ```

2. **Wait for Init:**
   ```
   [I][panasonic_ac]: Panasonic AC component v0.9 initialized
   ```

3. **Toggle Switch in Home Assistant**

4. **Look for Complete Sequence:**
   ```
   [D][switch:020]: 'Panasonic AC Powerful' Turning ON.
   [D][panasonic_ac.dnskp11]: Turning powerful mode on      ← This line!
   [V][panasonic_ac:209]: TX: 5A.XX.10.08...                ← Command sent
   [V][panasonic_ac:211]: RX: 5A.XX.10.88...                ← ACK received
   [D][climate:413]: Custom Preset: Powerful                ← State updated
   ```

### Expected Behavior

1. **Turn ON Powerful:**
   - Log: "Turning powerful mode on"
   - Sends: 0xB2 = 0x42 (Powerful preset)
   - If Quiet was on, it turns off automatically
   - Climate preset changes to "Powerful"

2. **Turn ON Quiet:**
   - Log: "Turning quiet mode on"
   - Sends: 0xB2 = 0x43 (Quiet preset)
   - If Powerful was on, it turns off automatically
   - Climate preset changes to "Quiet"

3. **Turn OFF Either:**
   - Log: "Turning X mode off"
   - Sends: 0xB2 = 0x41 (Normal preset)
   - Climate preset changes to "Normal"

### State Synchronization

The switches automatically sync with the preset mode:

- **Using Climate Preset:**
  - Set preset to "Powerful" → Powerful switch turns ON
  - Set preset to "Quiet" → Quiet switch turns ON
  - Set preset to "Normal" → Both switches turn OFF

- **Reports from AC:**
  - When AC reports preset 0x42 → Powerful switch ON
  - When AC reports preset 0x43 → Quiet switch ON
  - When AC reports preset 0x41 → Both switches OFF

### Manual Testing

```bash
# Enable VERBOSE logging
esphome logs your-device.yaml

# In Home Assistant:
# 1. Turn ON Powerful switch
# 2. Check logs for "Turning powerful mode on"
# 3. Verify preset shows "Powerful"
# 4. Turn ON Quiet switch
# 5. Check logs for:
#    - "Turning quiet mode on"
#    - Powerful switch turns OFF automatically
```

### Common Issues

**Issue:** Switches work but no logs
- **Cause:** Logger level too high (INFO/WARN/ERROR)
- **Fix:** Set `logger: level: DEBUG` or `VERBOSE`

**Issue:** Logs appear but AC doesn't respond
- **Cause:** Communication issue with AC
- **Fix:** Check UART wiring, baud rate, parity settings

**Issue:** Switch toggles back immediately
- **Cause:** AC rejecting the command
- **Fix:** Check AC is powered on and initialized

**Issue:** Both switches can be ON simultaneously
- **Cause:** Old code version without mutual exclusion
- **Fix:** Update to latest commit (includes mutual exclusion logic)

## NanoeX Switch

The NanoeX switch operates independently and logs at VERBOSE level:

```
[V][panasonic_ac.dnskp11]: Turning nanoex on
[V][panasonic_ac.dnskp11]: Turning nanoex off
```

You need `logger: level: VERBOSE` to see NanoeX log messages.

