# Panasonic AC Protocol Analysis and Bug Fixes

## Summary

Analyzed UART traffic captures from genuine Panasonic WiFi controller and identified **critical bugs** preventing proper communication with the AC unit, particularly affecting temperature reading.

## Critical Bugs Found

### 1. **HEADER MISMATCH (CRITICAL)**
**Problem:** Code only accepted packets with header `0x5A`, but AC unit sends packets with **BOTH** `0x5A` and `0x3A` headers.

**Evidence from captures:**
- Controller → AC: Always uses `0x5A` header
- AC → Controller: Uses **`0x3A` header** for many response packets (Reports, ACKs)

**Impact:** All packets with `0x3A` header were being **dropped silently**, causing:
- No temperature readings received
- No state updates
- No acknowledgment of commands
- Communication appearing broken

**Fix Applied:**
```cpp
// Before (panasonic_ac_component.cpp:264)
if (this->rx_buffer_[0] != HEADER)  // Only accepts 0x5A
{
  ESP_LOGW(TAG, "Dropping invalid packet (header)");
}

// After
if (this->rx_buffer_[0] != HEADER_TX && this->rx_buffer_[0] != HEADER_RX)  // Accepts both 0x5A and 0x3A
{
  ESP_LOGW(TAG, "Dropping invalid packet (header: 0x%02X)", this->rx_buffer_[0]);
  return false;
}
```

### 2. **Query Response Packet Size Mismatch**
**Problem:** Code expected exactly 125 bytes for query response, but actual packets vary in size (observed 140+ bytes).

**Evidence:**
- Power-on capture shows query response of 140 bytes
- Strict size check caused valid packets to be rejected

**Fix Applied:**
```cpp
// Before (panasonic_ac_component.cpp:436)
if (this->rx_buffer_.size() != 125) {
  ESP_LOGW(TAG, "Received invalid query response");
  return;
}

// After
if (this->rx_buffer_.size() < 70) {  // Minimum size check instead
  ESP_LOGW(TAG, "Received invalid query response - too short (size: %d)", this->rx_buffer_.size());
  return;
}
```

### 3. **Missing Response Packet Type Handlers**
**Problem:** Code only handled `0x10 0xXX` packet types, but AC also sends `0x90 0xXX` response packets (bit 7 set indicates response).

**Evidence from captures:**
- Poll request: `0x10 0x09` → AC responds with: `0x90 0xC9` or `0x90 0x09`
- Query: `0x10 0x89` → AC responds with: `0x90 0x89` or `0x90 0xC9`
- Command ACK from AC: `0x90 0x88` (in addition to `0x10 0x88`)

**Fix Applied:**
Updated all packet type handlers to accept both `0x10` and `0x90` variants:
```cpp
// Query/Poll response handler
} else if ((this->rx_buffer_[2] == 0x10 || this->rx_buffer_[2] == 0x90) && 
           (this->rx_buffer_[3] == 0x89 || this->rx_buffer_[3] == 0xC9))

// Command ACK handler  
} else if ((this->rx_buffer_[2] == 0x10 || this->rx_buffer_[2] == 0x90) && 
           this->rx_buffer_[3] == 0x88)

// Report handler
} else if ((this->rx_buffer_[2] == 0x10 || this->rx_buffer_[2] == 0x90) && 
           this->rx_buffer_[3] == 0x0A)
```

## Protocol Analysis

### Packet Header Structure
```
Byte 0: Header
  - 0x5A = Controller → AC (outbound)
  - 0x3A = AC → Controller (inbound)
  - 0x66 = Sync packet

Byte 1: Packet counter (increments, wraps at 0xFE)

Byte 2-3: Packet type
  - 0x10 0x08 = Set command
  - 0x10 0x09 = Poll request
  - 0x10 0x0A = Report from AC
  - 0x10 0x88 = Command ACK
  - 0x10 0x89 = Query response
  - 0x90 0x09 = Poll response (0x10 | 0x80)
  - 0x90 0x88 = Command ACK response (0x10 | 0x80)
  - 0x90 0xC9 = Extended poll/query response
```

### Temperature Reading in Query Response
For query/poll response packets (140 bytes observed):
```
Byte [14]: Power state (0x30 = on, 0x31 = off)
Byte [18]: Mode (0x41=Auto, 0x42=Cool, 0x43=Heat, 0x44=Dry, 0x45=Fan)
Byte [22]: Target temperature (raw value, multiply by 0.5 for °C)
Byte [62]: Current/Inside temperature (°C)
Byte [66]: Outside temperature (°C)
```

Special temperature values:
- `0x7E` (126) = Sensor not available
- `0x7F` (127) = Sensor error
- `0xFF` (255) = Invalid reading

## Files Modified

1. **components/panasonic_ac/panasonic_ac_component.h**
   - Added `HEADER_TX` (0x5A) and `HEADER_RX` (0x3A) constants
   - Kept `HEADER` for backwards compatibility

2. **components/panasonic_ac/panasonic_ac_component.cpp**
   - Fixed `verify_packet()` to accept both headers
   - Updated `handle_packet()` to handle 0x90 response variants
   - Removed strict 125-byte size check for query responses
   - Added safety check for outside temperature reading
   - Updated `send_packet()` to use `HEADER_TX`

## Expected Results After Fix

1. ✅ AC packets with 0x3A header will be properly received
2. ✅ Temperature readings (inside and outside) will work correctly
3. ✅ State updates from AC will be processed
4. ✅ Command acknowledgments will be received
5. ✅ Poll responses will be handled properly

## Testing Recommendations

1. Monitor logs for "Dropping invalid packet (header:" messages - should not appear for 0x3A
2. Check that outside temperature sensor updates with correct value (28°C from your capture)
3. Verify inside temperature reading is accurate
4. Confirm state changes are reflected promptly
5. Test all AC functions (mode, temp, fan, swing, etc.)

## Additional Observations

From the power-on handshake capture:
- Handshake sequence matches current implementation
- The official controller follows same packet sequencing
- No additional handshake steps are missing
- Protocol is bidirectional with proper acknowledgments

The main issue was simply the header byte difference preventing packet reception.

