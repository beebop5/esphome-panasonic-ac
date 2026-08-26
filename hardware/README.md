# Panasonic AC Interface Board (rev G)

Carrier PCB for this ESPHome component: an ESP32-C3 SuperMini plus a BSS138
level-converter module, connecting to a Panasonic AC through **either** the
CN-WLAN (DNSK-P11) port or the CN-CNT (CZ-TACG1) port.

![front](board-front.svg)
![back](board-back.svg)

## Specs

| | |
|---|---|
| Size | 48.5 × 18.0 mm, 1.0 mm FR-4, 2 layers |
| MCU | ESP32-C3 SuperMini (soldered, USB-C at the board edge) |
| Level shifter | BSS138 2-channel module, 10.16 mm row spacing |
| D1 | 1N5819 Schottky (reverse protection + blocks USB 5 V back-feed into the AC) |
| C1 | 100 µF / 16 V radial, 2.0 mm pitch |
| UART | 9600 8E1 — TX = GPIO3, RX = GPIO4 |

## Connectors — fit ONE of J11 / CNT

| Ref | Port | Pinout |
|---|---|---|
| **J11** | CN-WLAN (DNSK-P11), 1.27 mm single-row right-angle, 8-pin | 1 = +5V · 2 = TX (from AC) · 3 = RX (to AC) · 4–7 = NC · 8 = GND |
| **CNT** | CN-CNT (CZ-TACG1), 5-pin 2.54 mm header (pigtail to the AC's JST-PA) | 1 = +5V · 2 = TX · 3 = RX · **4 = +12 V — NO CONNECT (isolated pad)** · 5 = GND |

Both connectors share the same nets; the component's `type: wlan` / `type: cnt`
setting selects the protocol.

> **Warning:** CN-CNT pin 4 carries +12 V from the AC. The pad is intentionally
> connected to nothing. Never bridge it.

## Expansion headers

| Ref | Pins | Level | Notes |
|---|---|---|---|
| **J1** (top edge) | IO1 · IO0 · GND · 3V3 | 3.3 V | Direct GPIO, bidirectional |
| **J2** (bottom edge) | IO10 · IO7 · GND · 5V | 5 V | Via the BSS138 module's divider channels — **input to the ESP only** (5 V → 3.3 V, unidirectional) |

## Board features

- 3 mm-deep notch at the connector edge: the J11 right-angle pins cantilever
  over it so the AC connector body drops through the board plane.
- Antenna keep-out (no pour/vias) at the ESP32 antenna end; expect to set
  `output_power: 8.5dB` in ESPHome regardless.
- Two 2.5 mm non-plated cable-tie holes flanking the notch for strain relief.
- Ground pour both sides, stitched; all through-hole for hand assembly.

## Ordering

Upload `panasonic-p11-revG-gerbers-pcbway.zip` to PCBWay. Parameters:
2 layers, 48.5 × 18 mm, **1.0 mm** FR-4 TG150, 1 oz, lead-free HASL (or ENIG),
green mask, white silk both sides. `gerbers/README.txt` (included in the zip)
flags the notch and chamfers as intentional so CAM does not query them.

## Files

- `panasonic-p11-revG.kicad_pcb` / `.kicad_pro` — KiCad 9 source (routed, DRC-clean)
- `gerbers/` — plotted fab outputs + fab notes
- `panasonic-p11-revG-gerbers-pcbway.zip` — ready-to-upload fab package
- `generate_placement.py` — regenerates the pre-route placement from code
- `check_silkscreen.py` — verifies silk text is on-board, non-overlapping, off-pad
- `board-front.svg` / `board-back.svg` — renders

## Assembly notes

1. Flash the ESP32 over USB-C **before** fitting it if you plan to heat-shrink
   the assembly; verify OTA works before sealing.
2. Check the delivered BSS138 module: if it has an onboard 3.3 V LDO, do not
   feed `LV` from the ESP's 3V3 (see the AliExpress listing warning in the
   project docs).
3. Verify J11 pin 1 (+5 V) against your unit with a meter before first power-up.
4. Trim all leads flush on the back — the board sits against the AC control board.

Isolate mains at the breaker before opening the AC unit.
