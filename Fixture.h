// Fixture.h
// Named DMX values for the ADJ Stinger II 9-channel mode, and a thin Fixture
// class that maps semantic setters onto DMXSerial's channel buffer.
//
// Channel map (1-indexed, relative to the fixture's DMX start address):
//   1 Show Mode    : 0-9 off, 10-219 fixed shows 1-6, 220-255 random show
//   2 Color Macro  : 0-9 off, 10-198 color jump, 199-225 fade 1, 226-255 fade 2
//   3 LED Strobe   : 0-9 off, 10-244 slow->fast, 245-255 sound-controlled
//   4 UV Effect    : 0-134 off, 135-255 UV chase on
//   5 UV Chase     : 0-127 speed slow->fast, 128-255 speed slow->fast + strobing
//   6 Laser Colors : 0-9 off, 10-49 red, 50-89 green, 90-129 red+green,
//                    130-169 red+green flicker, 170-209 green+red flicker,
//                    210-249 flicker sync, 250-255 flicker alternate
//   7 Laser Strobe : same encoding as channel 3
//   8 LED Rotation : 0-9 stop, 10-127 CW slow->fast, 128-255 CCW slow->fast
//   9 Laser Rotation/Pattern: 0-127 static pattern, 128-255 moving chase slow->fast
//
// Constants below are mid-range picks within each documented bracket. Tune them
// against real hardware using the serial console (see Console.h / README.md).
#pragma once

#include <Arduino.h>
#include <DMXSerial.h>
#include "Config.h"

namespace ShowMode {     // Channel 1
  constexpr uint8_t OFF    = 0;
  constexpr uint8_t SHOW1  = 25;
  constexpr uint8_t SHOW2  = 60;
  constexpr uint8_t SHOW3  = 95;
  constexpr uint8_t SHOW4  = 130;
  constexpr uint8_t SHOW5  = 165;
  constexpr uint8_t SHOW6  = 200;
  constexpr uint8_t RANDOM = 240;  // 220-255
}

namespace ColorMacro {   // Channel 2
  constexpr uint8_t OFF   = 0;
  constexpr uint8_t JUMP  = 100;  // 10-198
  constexpr uint8_t FADE1 = 210;  // 199-225
  constexpr uint8_t FADE2 = 240;  // 226-255
}

// Shared by Channel 3 (LED Strobe) and Channel 7 (Laser Strobe).
namespace Strobe {
  constexpr uint8_t OFF    = 0;
  constexpr uint8_t SLOW   = 60;   // 10-244, slow->fast
  constexpr uint8_t MEDIUM = 140;
  constexpr uint8_t FAST   = 230;
  constexpr uint8_t SOUND  = 250;  // 245-255
}

namespace UV {           // Channel 4
  constexpr uint8_t OFF = 0;
  constexpr uint8_t ON  = 200;  // 135-255
}

namespace UVChase {      // Channel 5
  constexpr uint8_t OFF         = 0;
  constexpr uint8_t SLOW        = 40;   // 0-127, speed only
  constexpr uint8_t FAST        = 110;
  constexpr uint8_t STROBE_SLOW = 150;  // 128-255, speed + strobe
  constexpr uint8_t STROBE_FAST = 230;
}

namespace Laser {         // Channel 6
  constexpr uint8_t OFF                  = 0;
  constexpr uint8_t RED                  = 30;   // 10-49
  constexpr uint8_t GREEN                = 70;   // 50-89
  constexpr uint8_t RED_GREEN            = 110;  // 90-129
  constexpr uint8_t RG_FLICKER           = 150;  // 130-169
  constexpr uint8_t GR_FLICKER           = 190;  // 170-209
  constexpr uint8_t RG_FLICKER_SYNC      = 230;  // 210-249
  constexpr uint8_t RG_FLICKER_ALTERNATE = 252;  // 250-255
}

// Channel 7 (Laser Strobe) reuses the Strobe:: constants above.

namespace LedRotation {   // Channel 8
  constexpr uint8_t STOP     = 0;    // 0-9
  constexpr uint8_t CW_SLOW  = 30;   // 10-127, CW
  constexpr uint8_t CW_FAST  = 120;
  constexpr uint8_t CCW_SLOW = 140;  // 128-255, CCW
  constexpr uint8_t CCW_FAST = 250;
}

namespace LaserRotation { // Channel 9
  constexpr uint8_t STATIC_A     = 30;   // 0-127, static pattern
  constexpr uint8_t STATIC_B     = 90;
  constexpr uint8_t CHASE_SLOW   = 140;  // 128-255, moving chase
  constexpr uint8_t CHASE_MEDIUM = 200;
  constexpr uint8_t CHASE_FAST   = 250;
}

// Maps semantic setters onto 9 consecutive DMX channels starting at baseAddress.
// Reads/writes go directly through DMXSerial's own buffer - no local cache.
class Fixture {
public:
  explicit Fixture(uint16_t baseAddress) : _base(baseAddress) {}

  void setShowMode(uint8_t value)      { DMXSerial.write(_base + 0, value); }
  void setColorMacro(uint8_t value)    { DMXSerial.write(_base + 1, value); }
  void setLedStrobe(uint8_t value)     { DMXSerial.write(_base + 2, value); }
  void setUV(uint8_t value)            { DMXSerial.write(_base + 3, value); }
  void setUVChase(uint8_t value)       { DMXSerial.write(_base + 4, value); }
  void setLasers(uint8_t value)        { DMXSerial.write(_base + 5, value); }
  void setLaserStrobe(uint8_t value)   { DMXSerial.write(_base + 6, value); }
  void setLedRotation(uint8_t value)   { DMXSerial.write(_base + 7, value); }
  void setLaserRotation(uint8_t value) { DMXSerial.write(_base + 8, value); }

  // Writes 0 to all FIXTURE_CHANNEL_COUNT channels.
  void blackout();

  uint16_t baseAddress() const { return _base; }

private:
  uint16_t _base;
};
