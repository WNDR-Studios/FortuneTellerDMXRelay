// Show.h
// The "trippy" laser sequence as a table of steps. This is the file to edit when
// designing/tuning the show - see README.md for the field reference and the
// rotation-must-keep-moving rule.
#pragma once

#include <Arduino.h>
#include "Fixture.h"

struct ShowStep {
  uint8_t showMode;
  uint8_t colorMacro;
  uint8_t ledStrobe;
  uint8_t uv;
  uint8_t uvChase;
  uint8_t lasers;
  uint8_t laserStrobe;
  uint8_t ledRotation;
  uint8_t laserRotation;
  uint16_t durationMs;
};

inline void applyShowStep(Fixture &fx, const ShowStep &s) {
  fx.setShowMode(s.showMode);
  fx.setColorMacro(s.colorMacro);
  fx.setLedStrobe(s.ledStrobe);
  fx.setUV(s.uv);
  fx.setUVChase(s.uvChase);
  fx.setLasers(s.lasers);
  fx.setLaserStrobe(s.laserStrobe);
  fx.setLedRotation(s.ledRotation);
  fx.setLaserRotation(s.laserRotation);
}

// IMPORTANT:
//   showMode MUST be ShowMode::OFF (0) in every step. Any non-zero value
//   activates the fixture's internal auto-show, which ignores channels 2-9.
//   keep ledRotation outside LedRotation::STOP's range (0-9) so the head
//   keeps moving. LaserRotation values: verify 0-127 vs 128-255 behavior
//   on real hardware before changing (see Fixture.h channel 9 note).
const ShowStep showSequence[] = {
  // showMode,       colorMacro,         ledStrobe,       uv,      uvChase,
  //   lasers,                laserStrobe,    ledRotation,          laserRotation,        durationMs
  { ShowMode::OFF,  ColorMacro::JUMP,   Strobe::OFF,     UV::ON,  UVChase::SLOW,
    Laser::RED_GREEN,      Strobe::SLOW,    LedRotation::CW_SLOW,  LaserRotation::CHASE_MEDIUM,  1500 },

  { ShowMode::OFF,  ColorMacro::JUMP,   Strobe::MEDIUM,  UV::ON,  UVChase::FAST,
    Laser::RG_FLICKER,     Strobe::MEDIUM,  LedRotation::CCW_SLOW, LaserRotation::CHASE_MEDIUM,  1200 },

  { ShowMode::OFF,  ColorMacro::FADE1,  Strobe::FAST,    UV::ON,  UVChase::STROBE_SLOW,
    Laser::RG_FLICKER_SYNC, Strobe::FAST,  LedRotation::CW_FAST,  LaserRotation::CHASE_FAST,    1000 },

  { ShowMode::OFF,  ColorMacro::FADE2,  Strobe::MEDIUM,  UV::ON,  UVChase::FAST,
    Laser::GR_FLICKER,     Strobe::MEDIUM,  LedRotation::CCW_FAST, LaserRotation::CHASE_MEDIUM,  1200 },
};

constexpr size_t showSequenceLength = sizeof(showSequence) / sizeof(showSequence[0]);
