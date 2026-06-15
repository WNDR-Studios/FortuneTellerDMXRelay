# Project: DMX Laser Trigger Sequence

## Hardware

- **Board:** Arduino Mega 2560 (R3)
- **DMX Shield:** CQRobot DMX Shield (MAX485-based, Neutrik XLR in/out, jumper-configurable)
  - Use jumpers to route DMX TX via Serial1 (not pins 0/1) to avoid conflicts with USB programming
  - Master mode jumper enabled
- **Fixtures:** 2x ADJ Stinger II (moonflower/strobe/laser), set to 9-channel DMX mode
  - Both fixtures on same DMX address (mirrored) unless noted otherwise
- **Trigger input:** 120VAC coil relay (DPDT, e.g. mxuteuk JQX-13FL) isolating a 120VAC
  signal from the machine's "fortune teller" button. Relay's dry contacts switch a 5V
  logic signal into a digital input pin (with pull-down resistor).
  - Pin HIGH = trigger active → run laser sequence
  - Pin LOW = trigger inactive → blackout

## DMX Library

Use [DMXSerial by Matthias Hertel](https://github.com/mathertel/DMXSerial), configured
for Serial1 on the Mega.

## ADJ Stinger II — 9-Channel DMX Map

| Ch | Function | Notes |
|----|----------|-------|
| 1 | Show Mode | 220–255 = random built-in show |
| 2 | Color Macro | 10–198 = color jump |
| 3 | LED Strobe | 10–244 = slow→fast |
| 4 | UV LEDs | 135–255 = UV chase on |
| 5 | UV Chase Speed/Strobe | 0–127 slow→fast, 128–255 strobing |
| 6 | Lasers | 10–49 red, 50–89 green, 90–129 red+green, 130+ laser strobe |
| 7 | Laser Strobe | speed |
| 8 | LED Rotation | motor speed/direction |
| 9 | Laser Rotation | motor speed/direction |

Each fixture occupies 9 consecutive channels starting at its set DMX address.

## Behavior Requirements

1. On trigger HIGH: ramp into a "trippy" laser sequence (lasers, rotation, strobe,
   random show mode) for a fixed/short duration.
2. On trigger LOW: send explicit all-zero values across all used channels for ~500ms
   *before* halting/holding output. Do not just cut the DMX stream abruptly — this
   causes Stinger II fixtures to snap into random/auto states.
3. Avoid pointing lasers at fixed positions for long periods (rotation should keep moving
   during active sequence).

## Build/Test Notes

- Arduino IDE or arduino-cli for compiling/uploading
- DMX output only verifiable with real hardware or a DMX tester/sniffer — no software sim
- Keep 120VAC wiring and 5V logic wiring conceptually separate in any wiring diagrams or
  pin-mapping docs generated
