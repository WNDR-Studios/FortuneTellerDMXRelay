# Wiring Diagram — FortuneTellerDMXRelay

## System Overview

```
  120VAC Machine                      5V Logic Side
  ─────────────────                   ──────────────────────────────────────
  Fortune Teller                      Arduino Mega 2560
  Button Signal ──► Relay coil        TRIGGER_PIN (D3) ◄── Relay NO contact
  (hot leg)         (A1 / A2)         GND              ──► Relay COM contact
                        │             (internal pull-up holds D3 HIGH at rest;
                    Relay DPDT         relay closing pulls D3 LOW = trigger active)
                    (JQX-13FL)
                        │
                    Dry contact
                    (NO / COM)
```

---

## 1. Power Rails

| Source | Connects To | Notes |
|--------|-------------|-------|
| USB or 7-12V barrel jack | Arduino Mega VIN / USB | Powers Mega + shield logic |
| Separate 5V supply (or Mega 5V pin) | Pull-down resistor rail | Keep current draw under 40 mA per Mega output |
| Stinger II IEC inlet | Mains AC | Fixtures power from wall; fully independent of Arduino |

---

## 2. mxuteuk JQX-13FL Relay (120VAC coil, DPDT)

The relay isolates the 120VAC "fortune teller" button signal from the 5V Arduino logic.

### Relay pinout (screw terminals)

```
  ┌──────────────────────────────────┐
  │  A1   A2   │  C1  NO1  NC1  │  C2  NO2  NC2  │
  └──────────────────────────────────┘
       Coil        Pole 1 contacts    Pole 2 contacts
```

| Terminal | Function | Connect To |
|----------|----------|------------|
| A1 | Coil + | 120VAC hot leg from machine button signal |
| A2 | Coil − | 120VAC neutral (machine side) |
| C1 (COM) | Pole 1 common | Arduino GND |
| NO1 | Pole 1 normally open | Arduino D3 (TRIGGER_PIN) |
| NC1 | Pole 1 normally closed | Leave unconnected |
| C2 / NO2 / NC2 | Pole 2 | Leave unconnected (only one pole needed) |

> Only use the dry (low-voltage) contacts on one pole. Never let 120VAC reach
> any Arduino pin.

### Pull-up (internal) — no external resistor needed

```
  D3 (TRIGGER_PIN) ──── NO1 (relay)
  [INPUT_PULLUP]
  
  GND ──── C1 / COM (relay)
```

- Relay de-energized (button not pressed): NO1 is open → D3 held HIGH by the Mega's internal pull-up (~40 kΩ).
- Relay energized (button pressed): NO1 closes → D3 pulled LOW through GND → trigger active.
- `pinMode(TRIGGER_PIN, INPUT_PULLUP)` in `setup()` enables the internal resistor; no external component required.

---

## 3. Arduino Mega 2560 Pin Assignments

| Pin | Direction | Function |
|-----|-----------|----------|
| D2 | Output | DMX_MODE_PIN — MAX485 DE/RE direction control (CQRobot shield) |
| D3 | Input | TRIGGER_PIN — relay NO contact; INPUT_PULLUP, active-LOW (relay COM → GND) |
| D18 (TX1) | Output | DMX data out to shield (Serial1, jumper-selected on shield) |
| D19 (RX1) | Input | DMX data in from shield (Serial1, unused in controller mode) |
| 5V | Power | Available for pull-down resistor rail if needed |
| GND | Ground | Common ground for relay dry contact and pull-down resistor |
| USB / VIN | Power in | Programming and operating power |

---

## 4. CQRobot DMX Shield

The shield stacks directly on the Mega headers. Two jumper settings are required:

| Jumper | Setting | Purpose |
|--------|---------|---------|
| Serial port select | Serial1 (pins 18/19) | Route DMX through TX1/RX1, not TX0/RX0, to keep USB free |
| Master/Slave | Master | Enable transmit mode (DE/RE driven by D2) |

DMX output appears on the shield's Neutrik XLR-5 female connector:

| XLR Pin | DMX Signal |
|---------|-----------|
| 1 | Shield / GND |
| 2 | DMX− (Data −) |
| 3 | DMX+ (Data +) |
| 4 | (unused) |
| 5 | (unused) |

---

## 5. DMX Chain to Fixtures

```
  CQRobot Shield                 ADJ Stinger II #1              ADJ Stinger II #2
  XLR-5 OUT (female) ──5-pin──► XLR-5 IN (female)  ──5-pin──► XLR-5 IN (female)
                                 XLR-5 OUT (female) ──────────► (daisy chain thru)
                                 DMX address: 001               DMX address: 001
```

- Both fixtures set to DMX address **001** (dip switches on fixture) — they mirror each other.
- Use DMX 5-pin XLR cables. Standard 3-pin XLR adapters work if you pin-match 1/2/3.
- Terminate the last fixture with a 120 Ω resistor across DMX+/DMX− if the run exceeds a few meters or you see glitching.

---

## 6. Full Wiring Diagram (ASCII)

```
  ══════════════════════════════ 120VAC SIDE ═══════════════════════════════

  Machine 120VAC button signal
  HOT leg ──────────────────────────────────────► A1 ┐
                                                      │  JQX-13FL relay coil
  NEUTRAL ──────────────────────────────────────► A2 ┘

  ═════════════════════════════ 5V LOGIC SIDE ══════════════════════════════

  JQX-13FL relay dry contacts (Pole 1)
                                    NO1 ──────────────────────────────► D3 (TRIGGER_PIN, INPUT_PULLUP)
                                    C1  ──────────────────────────────► GND (Arduino)

  No external resistor needed — INPUT_PULLUP holds D3 HIGH at rest.
  Relay energized: NO1 closes → D3 → GND → LOW = trigger active.

  Arduino Mega 2560
  ┌──────────────────────────────────────────────────────┐
  │  D2  ──────────────────────────────────────────────► MAX485 DE/RE (via shield header) │
  │  D3  ◄──── relay NO1  (INPUT_PULLUP, active-LOW)                                      │
  │  D18 (TX1) ──────────────────────────────────────► CQRobot shield Serial1 TX          │
  │  D19 (RX1) ◄────────────────────────────────────── CQRobot shield Serial1 RX          │
  │  GND ──────────────────────────────────────────────► relay C1 / shield GND            │
  └──────────────────────────────────────────────────────┘
            │ (stacked)
  CQRobot DMX Shield
  ┌──────────────────────────────────────────────────────┐
  │  Jumpers: Serial1 selected, Master mode enabled       │
  │  XLR-5 OUT ──────────────────────────────────────────┼──► DMX cable to Stinger II #1 │
  └──────────────────────────────────────────────────────┘

  DMX Chain (5-pin XLR)
  Shield OUT ──► Stinger #1 IN ──► Stinger #1 OUT ──► Stinger #2 IN ──[120Ω terminator]
                 (addr 001)                           (addr 001)
```

---

## 7. Safety Notes

- The relay coil side (A1/A2) carries 120VAC. Use appropriate wire gauge, strain relief, and enclosure. Do not route 120VAC wiring near the Arduino or shield.
- Verify with a multimeter that D3 reads ~5V at rest (pull-up) and ~0V when the machine button is pressed (relay energized), **before** connecting the DMX shield or fixtures.
- The Arduino's GND and the relay's C1/COM are the only electrical connection between the two voltage domains. This is intentional — the relay provides the isolation.
- Use a fused spur or inline fuse on the 120VAC feed to the relay coil.
