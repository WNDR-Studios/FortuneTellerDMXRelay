// Config.h
// Pin assignments, DMX addressing, and timing constants.
// Verify TRIGGER_PIN, FIXTURE_ADDRESS and DMX_MAX_CHANNEL against your actual wiring
// and the DMX address dip-switches on both Stinger II units (see README.md).
#pragma once

#include <Arduino.h>

// ---- Trigger input ----
// Digital input from the isolation relay (INPUT_PULLUP). Internal pull-up holds this
// HIGH at rest; relay closing pulls it LOW through GND. LOW = run the show, HIGH = blackout.
constexpr uint8_t TRIGGER_PIN = 3;

// ---- DMX ----
// Direction-control (DE/RE) pin for the MAX485 transceiver. Matches both the
// DMXSerial library default (DMXMODEPIN) and the CQRobot shield's jumper default.
constexpr uint8_t DMX_MODE_PIN = 2;

// DMX start address shared by both Stinger II fixtures (mirrored on one address).
constexpr uint16_t FIXTURE_ADDRESS = 1;

// Number of consecutive DMX channels used by the Stinger II in 9-channel mode.
constexpr uint8_t FIXTURE_CHANNEL_COUNT = 9;

// Highest DMX channel transmitted. A little headroom above FIXTURE_ADDRESS + 8.
constexpr uint16_t DMX_MAX_CHANNEL = 16;

// ---- Timing ----
// How long to hold an explicit all-zero DMX frame after the trigger goes LOW,
// before settling into the idle (still all-zero) state. Prevents the Stinger II
// fixtures from snapping into a random/auto state when DMX data simply stops.
constexpr unsigned long BLACKOUT_HOLD_MS = 500;

// ---- Console ----
// Interactive serial console for live DMX exploration (see Console.h). Uses
// Serial2 (pin 16=TX2, pin 17=RX2) — connect a USB-to-serial adapter there.
// Serial0 (pins 0/1) is reserved for the DMX shield and must stay free.
// Set to false for a deployed build with no adapter attached.
constexpr bool ENABLE_DMX_CONSOLE = false;
constexpr unsigned long SERIAL_BAUD = 115200;
