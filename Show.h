// Show.h
// Eight shows, each built around one or two active light elements so nothing
// competes. The Stinger II has three independent elements:
//   - LED/moonflower head  (ch2 color macro, ch3 strobe, ch8 rotation)
//   - UV LEDs              (ch4 on/off, ch5 chase speed/strobe)
//   - Lasers               (ch6 color, ch7 strobe, ch9 rotation)
//
// Setting ColorMacro::OFF kills the moonflower head entirely.
// Setting UV::OFF kills the UV LEDs entirely.
// Most shows below keep one or two elements active so each has a clear identity.
//
// To add a show:
//   1. Define a new `static const ShowStep show_name[] = { ... };` array.
//   2. Add it to allShows[] and allShowLengths[].
//   NUM_SHOWS updates automatically.
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

// ----------------------------------------------------------------------------
// 1. Red Drift — red laser with a slow color fade that breathes in and out
//    every 2 seconds. Moonflower head on for 2s, off for 2s, repeat.
// ----------------------------------------------------------------------------
static const ShowStep show_red_drift[] = {
  { ShowMode::OFF, ColorMacro::FADE2, Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::RED,  Strobe::OFF, LedRotation::CW_SLOW, LaserRotation::CHASE_SLOW, 2000 },
  { ShowMode::OFF, ColorMacro::OFF,   Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::RED,  Strobe::OFF, LedRotation::CW_SLOW, LaserRotation::CHASE_SLOW, 2000 },
};

// ----------------------------------------------------------------------------
// 2. Emerald — green laser with the same slow color breath.
// ----------------------------------------------------------------------------
static const ShowStep show_emerald[] = {
  { ShowMode::OFF, ColorMacro::FADE2, Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::GREEN, Strobe::OFF, LedRotation::CCW_SLOW, LaserRotation::CHASE_SLOW, 2000 },
  { ShowMode::OFF, ColorMacro::OFF,   Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::GREEN, Strobe::OFF, LedRotation::CCW_SLOW, LaserRotation::CHASE_SLOW, 2000 },
};

// ----------------------------------------------------------------------------
// 3. Twin Beams — both lasers with the slow color breath.
// ----------------------------------------------------------------------------
static const ShowStep show_twin_beams[] = {
  { ShowMode::OFF, ColorMacro::FADE2, Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::RED_GREEN, Strobe::OFF, LedRotation::CW_MEDIUM, LaserRotation::CHASE_MEDIUM, 2000 },
  { ShowMode::OFF, ColorMacro::OFF,   Strobe::OFF, UV::OFF, UVChase::OFF,
    Laser::RED_GREEN, Strobe::OFF, LedRotation::CW_MEDIUM, LaserRotation::CHASE_MEDIUM, 2000 },
};

// ----------------------------------------------------------------------------
// 4. UV + Red — UV atmosphere added to a red laser. No moonflower head.
// ----------------------------------------------------------------------------
static const ShowStep show_uv_red[] = {
  { ShowMode::OFF, ColorMacro::OFF, Strobe::OFF, UV::ON, UVChase::SLOW,
    Laser::RED,   Strobe::OFF, LedRotation::CW_SLOW, LaserRotation::CHASE_SLOW, 8000 },
};

// ----------------------------------------------------------------------------
// 5. UV + Green — UV atmosphere with green laser. Eerier than UV + Red.
// ----------------------------------------------------------------------------
static const ShowStep show_uv_green[] = {
  { ShowMode::OFF, ColorMacro::OFF, Strobe::OFF, UV::ON, UVChase::SLOW,
    Laser::GREEN, Strobe::OFF, LedRotation::CCW_SLOW, LaserRotation::CHASE_SLOW, 8000 },
};

// ----------------------------------------------------------------------------
// 6. UV Pulse — UV strobing is the main event; green laser holds steady.
// ----------------------------------------------------------------------------
static const ShowStep show_uv_pulse[] = {
  { ShowMode::OFF, ColorMacro::OFF, Strobe::OFF, UV::ON, UVChase::STROBE_SLOW,
    Laser::GREEN, Strobe::OFF, LedRotation::CCW_SLOW, LaserRotation::CHASE_MEDIUM, 8000 },
};

// ----------------------------------------------------------------------------
// 7. Flicker — UV on + red+green laser flicker. Two steps: flicker on, off.
// ----------------------------------------------------------------------------
static const ShowStep show_flicker[] = {
  { ShowMode::OFF, ColorMacro::OFF, Strobe::OFF, UV::ON, UVChase::SLOW,
    Laser::RG_FLICKER, Strobe::OFF, LedRotation::CW_SLOW, LaserRotation::CHASE_MEDIUM, 4500 },

  { ShowMode::OFF, ColorMacro::OFF, Strobe::OFF, UV::ON, UVChase::SLOW,
    Laser::RED_GREEN,  Strobe::OFF, LedRotation::CW_SLOW, LaserRotation::CHASE_MEDIUM, 4500 },
};

// ----------------------------------------------------------------------------
// 8. Full Color — everything on. Moonflower + UV + both lasers. The big show.
//    One gentle strobe on the LED head for drama.
// ----------------------------------------------------------------------------
static const ShowStep show_full_color[] = {
  { ShowMode::OFF, ColorMacro::FADE2, Strobe::SLOW, UV::ON, UVChase::SLOW,
    Laser::RED_GREEN, Strobe::OFF, LedRotation::CW_MEDIUM, LaserRotation::CHASE_MEDIUM, 8000 },
};

// ----------------------------------------------------------------------------
// Registry — add new shows here; NUM_SHOWS updates automatically
// ----------------------------------------------------------------------------
static const ShowStep * const allShows[] = {
  show_red_drift,
  show_emerald,
  show_twin_beams,
  show_uv_red,
  show_uv_green,
  show_uv_pulse,
  show_flicker,
  show_full_color,
};

static const size_t allShowLengths[] = {
  sizeof(show_red_drift)   / sizeof(show_red_drift[0]),
  sizeof(show_emerald)     / sizeof(show_emerald[0]),
  sizeof(show_twin_beams)  / sizeof(show_twin_beams[0]),
  sizeof(show_uv_red)      / sizeof(show_uv_red[0]),
  sizeof(show_uv_green)    / sizeof(show_uv_green[0]),
  sizeof(show_uv_pulse)    / sizeof(show_uv_pulse[0]),
  sizeof(show_flicker)     / sizeof(show_flicker[0]),
  sizeof(show_full_color)  / sizeof(show_full_color[0]),
};

constexpr uint8_t NUM_SHOWS = sizeof(allShows) / sizeof(allShows[0]);
