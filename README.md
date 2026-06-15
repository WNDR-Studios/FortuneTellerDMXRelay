# FortuneTellerDMXRelay

Trigger-driven DMX laser show controller for an arcade "fortune teller" cabinet.
An Arduino Mega 2560 with a CQRobot DMX shield drives two ADJ Stinger II
moonflower/strobe/laser fixtures (mirrored on one DMX address). A relay-isolated
digital input tells the sketch when to run the show and when to black out.

Full hardware/wiring details (120VAC relay isolation, jumper settings, fixture
DMX modes) are in [`CLAUDE.md`](CLAUDE.md). This README covers building,
running, designing the show, and live-tuning fixture values.

## Files

| File | Purpose |
|---|---|
| `FortuneTellerDMXRelay.ino` | `setup()`/`loop()` and the trigger state machine |
| `Config.h` | Pins, DMX address, timing, console toggle |
| `Fixture.h` / `Fixture.cpp` | Named Stinger II channel values + `Fixture` class |
| `Show.h` | The show sequence table - **edit this to design the show** |
| `Console.h` / `Console.cpp` | Interactive serial console for live DMX tuning |

## 1. DMX Library Setup (Serial1 on the Mega)

This project uses [DMXSerial by Matthias Hertel](https://github.com/mathertel/DMXSerial).
On an Arduino Mega 2560, DMXSerial defaults to hardware Serial0 (pins 0/1) - but
this project's shield is wired to **Serial1** (pins 18/19) to avoid conflicts
with USB programming. The library has a Serial1 mode (`DMX_USE_PORT1`) that is
disabled by default.

**Recommended: pass a compile flag, don't edit the library.** The flag
`-DDMX_USE_PORT1` applies to every compiled file, including the library, so it
enables Serial1 mode without touching the installed library source:

```sh
arduino-cli compile --fqbn arduino:avr:mega \
  --build-property "compiler.cpp.extra_flags=-DDMX_USE_PORT1" .
```

**Arduino IDE fallback:** if you're not using arduino-cli, open the installed
copy of `DMXSerial/src/DMXSerial_avr.h` (under your Arduino `libraries` folder)
and uncomment:

```cpp
#define DMX_USE_PORT1
```

**Verify:** with a DMX tester/sniffer, confirm the DMX signal appears on the
Mega's pins 18 (TX1)/19 (RX1), not pins 0/1.

## 2. Build & Upload

```sh
arduino-cli core install arduino:avr
arduino-cli lib install DMXSerial

arduino-cli compile --fqbn arduino:avr:mega \
  --build-property "compiler.cpp.extra_flags=-DDMX_USE_PORT1" .

arduino-cli upload -p <PORT> --fqbn arduino:avr:mega .
```

Replace `<PORT>` with the Mega's serial port (e.g. `COM5`).

## 3. How It Works

The sketch is a small state machine driven by `TRIGGER_PIN` (`Config.h`):

- **IDLE** - trigger LOW. The DMX buffer holds all-zero values and DMXSerial
  keeps transmitting them continuously in the background.
- **ACTIVE** - trigger goes HIGH. The sketch steps through `showSequence`
  (`Show.h`) on a `millis()` timer, looping back to the start for as long as the
  trigger stays HIGH.
- **BLACKOUT** - trigger goes LOW. All 9 channels are immediately zeroed and
  held for `BLACKOUT_HOLD_MS` (default 500ms) before returning to IDLE.

The explicit zero-and-hold step matters: the Stinger II fixtures snap into a
random/auto state if the DMX signal simply stops, so the sketch always sends an
explicit all-zero frame for a short time before going quiet.

DMXSerial's controller mode transmits the buffer continuously via interrupts
once `DMXSerial.init(DMXController, ...)` is called - the sketch's only job is
to keep that buffer updated, never to start/stop the DMX stream itself.

## 4. Designing the Show (`Show.h`)

`Show.h` contains `showSequence[]`, an array of `ShowStep` entries. Each step
sets all 9 Stinger II channels and holds them for `durationMs` before moving to
the next step (looping back to the start while the trigger is HIGH):

```cpp
struct ShowStep {
  uint8_t showMode;      // Channel 1
  uint8_t colorMacro;    // Channel 2
  uint8_t ledStrobe;     // Channel 3
  uint8_t uv;            // Channel 4
  uint8_t uvChase;       // Channel 5
  uint8_t lasers;        // Channel 6
  uint8_t laserStrobe;   // Channel 7
  uint8_t ledRotation;   // Channel 8
  uint8_t laserRotation; // Channel 9
  uint16_t durationMs;   // how long to hold this step
};
```

To change the show: add, remove, reorder, or edit entries in `showSequence[]`,
using the named constants from `Fixture.h` (table below) for readability.

### Hard rule: keep rotation moving

Per the project spec, lasers must never sit in a fixed position for long during
the active sequence. Concretely, in **every** step:

- `ledRotation` must stay **outside** `LedRotation::STOP`'s range (DMX 0-9) -
  use a `CW_*` or `CCW_*` constant.
- `laserRotation` must stay **>= 128** (one of the `CHASE_*` constants) - never
  `STATIC_A`/`STATIC_B` (DMX 0-127, static patterns).

### Fixture.h constants reference

Mid-range picks within each documented DMX bracket for the Stinger II 9-channel
mode. These are starting points - use console mode (below) to tune them against
real fixtures, since units can vary slightly.

| Channel | Function | DMX range | Constants (namespace) |
|---|---|---|---|
| 1 | Show Mode | 0-9 off, 10-219 shows 1-6, 220-255 random | `ShowMode::OFF/SHOW1.. SHOW6/RANDOM` |
| 2 | Color Macro | 0-9 off, 10-198 jump, 199-225 fade1, 226-255 fade2 | `ColorMacro::OFF/JUMP/FADE1/FADE2` |
| 3 | LED Strobe | 0-9 off, 10-244 slow->fast, 245-255 sound | `Strobe::OFF/SLOW/MEDIUM/FAST/SOUND` |
| 4 | UV Effect | 0-134 off, 135-255 UV chase on | `UV::OFF/ON` |
| 5 | UV Chase Speed/Strobe | 0-127 speed only, 128-255 speed+strobe | `UVChase::OFF/SLOW/FAST/STROBE_SLOW/STROBE_FAST` |
| 6 | Laser Colors | 0-9 off, 10-49 red, 50-89 green, 90-129 red+green, 130-169/170-209 flicker variants, 210-249 sync, 250-255 alternate | `Laser::OFF/RED/GREEN/RED_GREEN/RG_FLICKER/GR_FLICKER/RG_FLICKER_SYNC/RG_FLICKER_ALTERNATE` |
| 7 | Laser Strobe | same encoding as channel 3 | `Strobe::*` (shared) |
| 8 | LED Rotation | 0-9 stop, 10-127 CW slow->fast, 128-255 CCW slow->fast | `LedRotation::STOP/CW_SLOW/CW_FAST/CCW_SLOW/CCW_FAST` |
| 9 | Laser Rotation/Pattern | 0-127 static pattern, 128-255 moving chase slow->fast | `LaserRotation::STATIC_A/STATIC_B/CHASE_SLOW/CHASE_MEDIUM/CHASE_FAST` |

## 5. Console Mode (live DMX tuning)

With `ENABLE_DMX_CONSOLE = true` in `Config.h` (the default), open the Serial
Monitor at `115200` baud after uploading to send live commands to the fixtures
through the same `Fixture` API the show uses - handy for working out which
values look best on your actual units before encoding them in `Show.h`.

Sending any command (other than `run`) puts the sketch into **override mode**:
the trigger state machine is paused and the buffer only changes when you send
another command. Send `run` to resume normal trigger-driven operation.

Commands (lowercase, space-separated):

| Command | Effect |
|---|---|
| `c <channel> <value>` | Raw `DMXSerial.write(channel, value)` (channel 1-16, value 0-255) |
| `show <name\|0-255>` | Channel 1 - `off show1 show2 show3 show4 show5 show6 random` |
| `color <name\|0-255>` | Channel 2 - `off jump fade1 fade2` |
| `strobe <name\|0-255>` | Channel 3 - `off slow medium fast sound` |
| `uv <name\|0-255>` | Channel 4 - `off on` |
| `uvchase <name\|0-255>` | Channel 5 - `off slow fast strobeslow strobefast` |
| `laser <name\|0-255>` | Channel 6 - `off red green redgreen rgflicker grflicker rgsync rgalt` |
| `laserstrobe <name\|0-255>` | Channel 7 - `off slow medium fast sound` |
| `ledrot <name\|0-255>` | Channel 8 - `stop cwslow cwfast ccwslow ccwfast` |
| `laserrot <name\|0-255>` | Channel 9 - `statica staticb chaseslow chasemedium chasefast` |
| `step <index>` | Apply `showSequence[index]` directly (preview a step without the trigger) |
| `blackout` | Zero all 9 channels |
| `dump` | Print the current value of channels 1-9 |
| `run` | Clear override, resume the trigger-driven state machine |
| `help` | Print the command list |

Every command accepts either one of the named tokens above or a raw `0-255`
value, e.g. `laser red` and `laser 30` are equivalent.

### Example session

```
> laser red
> ledrot cwslow
> laserrot chaseslow
> dump
ch 1: 0
ch 2: 0
ch 3: 0
ch 4: 0
ch 5: 0
ch 6: 30
ch 7: 0
ch 8: 30
ch 9: 140
> run
resuming trigger-driven show
```

Once you've found values you like, copy them into a `ShowStep` in `Show.h`
(adding a new named constant in `Fixture.h` first if it's a new value worth
naming).

For a deployed build where you don't need the console, set
`ENABLE_DMX_CONSOLE = false` in `Config.h` to skip Serial setup entirely.

## 6. Operational Notes / Troubleshooting

- **Trigger wiring**: `TRIGGER_PIN` should read LOW at rest (external
  pull-down) and HIGH only when the isolation relay is energized. Verify this
  with a multimeter before connecting DMX fixtures.
- **No software simulation**: DMX output can only be verified with real
  fixtures or a DMX tester/sniffer - there's no way to simulate this sketch.
- **DMX addressing**: both Stinger II units must be dip-switched to the DMX
  start address in `FIXTURE_ADDRESS` (`Config.h`, default `1`).
- **Re-triggering during blackout**: if the trigger goes HIGH again during the
  500ms blackout hold, the sketch finishes the hold first, then restarts the
  show from step 1 on the next loop iteration. This delay is not perceptible
  for the cabinet's button-press use case.
