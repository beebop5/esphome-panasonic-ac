# Panasonic AC UART Protocol Documentation

## Overview

The Panasonic AC units communicate via UART with the following parameters:
- **Baud Rate:** 9600
- **Parity:** EVEN
- **Data Bits:** 8
- **Stop Bits:** 1

## Packet Structure

### Basic Packet Format

```
[Header] [Counter] [Type1] [Type2] [Payload...] [Checksum]
  1 byte   1 byte   1 byte   1 byte   Variable     1 byte
```

### Header Byte (Byte 0)
- **0x5A**: Controller → AC (TX packets)
- **0x3A**: AC → Controller (RX packets from AC's perspective)
- **0x66**: Sync packet (special case)

**CRITICAL:** Code must accept BOTH 0x5A and 0x3A headers for proper bidirectional communication.

### Counter Byte (Byte 1)
- Increments with each packet sent
- Separate counters for TX and RX
- Wraps from 0xFE → 0x01 (0xFF is not used)
- Used to detect and correct shifted packet sequences

### Type Bytes (Bytes 2-3)

#### Response Bit (Byte 2)
- Bit 7 may be set to indicate response: `0x90` = `0x10 | 0x80`
- Examples:
  - `0x10 0x09` = Poll request
  - `0x90 0xC9` = Poll response (`0x10 | 0x80`, `0x09 | 0xC0`)

#### Common Packet Types

| Type1 | Type2 | Direction | Description |
|-------|-------|-----------|-------------|
| 0x01  | 0x01  | Both      | Ping request |
| 0x01  | 0x81  | Response  | Ping response |
| 0x10  | 0x08  | TX        | Set command (change settings) |
| 0x10  | 0x09  | TX        | Poll request |
| 0x10  | 0x0A  | RX        | Report (unsolicited state update) |
| 0x90  | 0x0A  | RX        | Report (with response bit) |
| 0x10  | 0x88  | RX        | Command acknowledgment |
| 0x90  | 0x88  | RX        | Command ACK (with response bit) |
| 0x10  | 0x89  | RX        | Query response |
| 0x90  | 0xC9  | RX        | Poll response |
| 0x10  | 0x8A  | TX        | Report acknowledgment |
| 0x11  | 0x01  | Both      | Alternative ping |
| 0x11  | 0x03  | RX        | Telemetry/status packet |
| 0x00  | 0xXX  | Both      | Handshake packets |
| 0x01  | 0x00  | Both      | Handshake packets |
| 0x01  | 0x80  | RX        | Handshake complete |

### Checksum (Last Byte)
Two's complement checksum:
```c
uint8_t checksum = 0;
for (each byte in packet) {
    checksum += byte;
}
checksum = (~checksum + 1);  // Two's complement
```

Valid packet: sum of all bytes (including checksum) = 0x00

## Payload Structures

### Set Command (0x10 0x08)

```
[Header] [Counter] [0x10] [0x08] [Size Header] [Size] [Type] [Type2] 
  0x5A     0xXX              4 bytes for size and type metadata

[Pair Counter Header] [Pair Count] [Key-Value Pairs...] [Checksum]
  0x30                 0x01         N * 4 bytes
  
```

#### Key-Value Pair Format (4 bytes each):
```
[Key] [0x01] [Value] [Flags]
```

**Common Keys:**
| Key  | Description | Values |
|------|-------------|--------|
| 0x80 | Power State | 0x30=On, 0x31=Off |
| 0xB0 | Mode | 0x41=Auto, 0x42=Cool, 0x43=Heat, 0x44=Dry, 0x45=Fan |
| 0x31 | Target Temp | Raw value * 0.5 = °C (e.g., 0x2E = 23°C) |
| 0xA0 | Fan Speed | 0x41=Auto, 0x32-0x36=Levels 1-5 |
| 0xB2 | Preset Mode | 0x41=Normal, 0x42=Powerful, 0x43=Quiet |
| 0xA1 | Swing Mode | 0x41=Both, 0x42=Off, 0x43=Vertical, 0x44=Horizontal |
| 0xA4 | Vertical Swing Position | 0x41=Up, 0x42=Down, 0x43=Mid, 0x44=MidUp, 0x45=MidDown |
| 0xA5 | Horizontal Swing Position | 0x41=Right, 0x42=Left, 0x43=Centre, 0x56=CentreRight, 0x5C=CentreLeft |
| 0x33 | NanoeX | 0x45=On, 0x42=Off |
| 0x34 | Unknown Mode Flag | Usually 0x42 |
| 0x35 | Eco Mode | 0x42=Off, 0x41=On |

### Query/Poll Response (0x10 0x89 / 0x90 0xC9)

Large packet containing full AC state (typically 125-140 bytes):

```
Offset | Description
-------|-------------
[0]    | Header (0x5A or 0x3A)
[1]    | Counter
[2-3]  | Type (0x10 0x89 or 0x90 0xC9)
[14]   | Power state (0x30=On, 0x31=Off)
[18]   | Mode (same as 0xB0 key)
[22]   | Target temperature (raw * 0.5)
[26]   | Fan speed
[30]   | Swing mode
[34]   | Horizontal swing position
[38]   | Vertical swing position
[42]   | Preset mode (0x41=Normal, 0x42=Powerful, 0x43=Quiet)
[50]   | NanoeX state
[62]   | Current/Inside temperature (°C)
[66]   | Outside temperature (°C)
```

**Temperature Special Values:**
- `0x7E` (126): Sensor not available
- `0x7F` (127): Sensor error
- `0xFF` (255): Invalid reading

### Report (0x10 0x0A / 0x90 0x0A)

Unsolicited state updates from AC:

```
[Header] [Counter] [0x10] [0x0A] [Size Header] [Size] [Type] [Type2]
[Pair Counter Header] [Pair Count] [Key-Value Pairs...] [Checksum]
```

Same key-value format as Set Command. Contains only changed values.

### Telemetry Packet (0x11 0x03)

Periodic diagnostic/status data (typically ~160 bytes):

```
[Header] [Counter] [0x11] [0x03] [Size] [Payload...] [Checksum]
  0x5A     0xXX                      Variable (160+ bytes)
```

**Payload Structure (tentative):**
- Multiple data blocks, each appears to contain:
  - Temperature readings
  - Sensor data
  - Operational metrics
  - Timestamps/counters

**Example packet breakdown:**
```
5A 84 11 03 00 A7 3C 01 A4 30 00 02 B8 E4 15 FD ...
│  │  │  │  │  │  │  Block of data repeats ~4 times
│  │  │  │  │  Size (0xA7 = 167 bytes?)
│  │  │  │  Unknown
│  │  │  Type2
│  │  Type1
│  Counter
Header
```

The packet appears to contain multiple similar blocks of ~40 bytes each, suggesting:
- Different zone/sensor readings
- Historical data points
- Multiple temperature measurements

## Handshake Sequence (16 Steps)

The initialization handshake establishes communication:

```
Step  Direction  Type        Description
----  ---------  ----------  -----------
1     CTRL → AC  0x00 0x06   Handshake init 1
2     CTRL → AC  0x00 0x09   Handshake init 2
      AC → CTRL  0x00 0x89   Handshake response 2
3     CTRL → AC  0x00 0x0C   Handshake 3
      AC → CTRL  0x00 0x8C   Handshake response 3
4     CTRL → AC  0x00 0x10   Handshake 4
      AC → CTRL  0x00 0x90   Handshake response 4
5     CTRL → AC  0x00 0x11   Handshake 5
      AC → CTRL  0x00 0x91   Handshake response 5
6     CTRL → AC  0x00 0x12   Handshake 6
      AC → CTRL  0x00 0x92   Handshake response 6
7     CTRL → AC  0x00 0x41   Handshake 7
      AC → CTRL  0x00 0xC1   Handshake response 7
8     CTRL → AC  0x01 0x4C   Handshake 8
      AC → CTRL  0x01 0xCC   Handshake response 8
9     CTRL → AC  0x10 0x00   Handshake 9
      AC → CTRL  0x10 0x80   Handshake response 9
10    CTRL → AC  0x10 0x01   Handshake 10 (with capabilities)
      AC → CTRL  0x10 0x81   Handshake response 10 (with AC info)
11    CTRL → AC  0x00 0x18   Handshake 11
      AC → CTRL  0x00 0x98   Handshake response 11
12    CTRL → AC  0x01 0x00   Handshake 12
      AC → CTRL  0x01 0x80   Handshake response 12
13    CTRL → AC  0x10 0x08   Handshake 13 (set command)
      AC → CTRL  0x10 0x88   Handshake ACK 13
14    AC → CTRL  0x01 0x09   Unsolicited (RX counter)
      CTRL → AC  0x01 0x89   Response 14 (with ID)
15    AC → CTRL  0x00 0x20   Unsolicited 2
      CTRL → AC  0x00 0xA0   Response 15 (with version)
      [First Poll happens here]
16    CTRL → AC  0x01 0x00   Final handshake
      AC → CTRL  0x01 0x80   Handshake complete
```

**Note:** AC may send ping packets (`0x01 0x01`) at any time during handshake.

## Communication Flow

### Normal Operation

```
┌─────────────┐                           ┌─────────────┐
│ Controller  │                           │  AC Unit    │
└──────┬──────┘                           └──────┬──────┘
       │                                         │
       │  Poll (0x10 0x09) every 30s            │
       │────────────────────────────────────────>│
       │                                         │
       │       Query Response (0x90 0xC9)       │
       │<────────────────────────────────────────│
       │                                         │
       │  Set Command (0x10 0x08)               │
       │────────────────────────────────────────>│
       │                                         │
       │       Command ACK (0x10 0x88)          │
       │<────────────────────────────────────────│
       │                                         │
       │       Report (0x10 0x0A)               │
       │<────────────────────────────────────────│
       │                                         │
       │  Report ACK (0x10 0x8A)                │
       │────────────────────────────────────────>│
       │                                         │
       │       Ping (0x01 0x01)                 │
       │<────────────────────────────────────────│
       │                                         │
       │  Ping Response (0x01 0x81)             │
       │────────────────────────────────────────>│
       │                                         │
       │    Telemetry (0x11 0x03) periodic      │
       │<────────────────────────────────────────│
       │                                         │
```

## Timing Parameters

- **Poll Interval:** 30000ms (30 seconds)
- **Response Timeout:** 600ms
- **Read Timeout:** 100ms (packet completion)
- **Init Timeout:** 10000ms (before starting handshake)
- **Init Fail Timeout:** 60000ms
- **First Poll Timeout:** 650ms
- **Init End Timeout:** 20000ms

## Error Handling

### Counter Synchronization
If packet counters drift out of sync, the code automatically corrects:
```
Expected TX counter: 0x10
Received: 0x0F
Action: Adjust RX counter to match
```

### Packet Resend
If no response within 600ms, resend last command (max 3 attempts implied).

### Checksum Validation
All packets must have valid checksums. Invalid packets are dropped and logged.

## Example Transactions

### Setting Temperature to 23°C
```
TX: 5A 2E 10 08 00 11 01 01 30 01 03 00 B0 01 62 02 91 01 0E 00 B2 01 41 D0
     │  │  │  │  └─ Size metadata
     │  │  │  └─ Set command
     │  │  └─ Type
     │  └─ Counter
     └─ Header (Controller → AC)

Breakdown:
  - Key 0xB0, Value 0x62 (mode?)
  - Key 0x91, Value 0x0E (related to temp?)
  - Key 0xB2, Value 0x41 (Normal preset)

RX: 3A 0E 90 88 00 0E 00 01 30 01 03 00 D0 00 02 31 00 00 B2 00 08
     │  └─ Note: 0x3A header (AC → Controller)
     └─ Must accept this header!
```

### Turning On Powerful Mode
```
TX: 5A XX 10 08 ... [0xB2][0x01][0x42][0x00] [0x35][0x01][0x42][0x00] ...
                     Powerful=On ────┘         Eco=Off ────────┘
```

## Important Notes

1. **Always handle both 0x5A and 0x3A headers** - This is critical for bidirectional communication
2. **Variable packet sizes** - Don't assume fixed sizes for query responses
3. **Response bit (0x80)** - Type byte 2 may have bit 7 set for responses
4. **Counter wrap** - Counters skip 0xFF, wrapping 0xFE → 0x01
5. **Ping packets** - Can occur at any time, including during handshake
6. **Telemetry is periodic** - Sent automatically by AC, no request needed
7. **Temperature scaling** - Target temp uses 0.5°C steps, current/outside temps are direct °C values

## References

- Based on analysis of real Panasonic WiFi controller UART captures
- Reverse engineered from esphome-panasonic-ac component
- Tested with Panasonic DNSKP11 module compatible AC units

