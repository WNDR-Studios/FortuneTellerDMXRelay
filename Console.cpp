// Console.cpp
//
// Command syntax: "<command> [arg1] [arg2]", one per line, lowercase.
// Most commands accept either a named token (e.g. "red") or a raw 0-255 value.
// See printHelp() below for the full list, or type 'help' in the console.
#include "Console.h"
#include "Show.h"
#include <DMXSerial.h>
#include <stdlib.h>
#include <string.h>

namespace {

bool overrideActive = false;

struct NamedValue {
  const char *name;
  uint8_t value;
};

bool lookup(const char *token, const NamedValue *table, uint8_t count, uint8_t &out) {
  for (uint8_t i = 0; i < count; i++) {
    if (strcmp(token, table[i].name) == 0) {
      out = table[i].value;
      return true;
    }
  }
  return false;
}

uint8_t resolveValue(const char *token, const NamedValue *table, uint8_t count) {
  uint8_t value;
  if (lookup(token, table, count, value)) {
    return value;
  }
  return (uint8_t)constrain(atoi(token), 0, 255);
}

template <uint8_t N>
uint8_t resolve(const char *token, const NamedValue (&table)[N]) {
  return resolveValue(token, table, N);
}

const NamedValue SHOW_MODE_NAMES[] = {
    {"off", ShowMode::OFF},     {"show1", ShowMode::SHOW1}, {"show2", ShowMode::SHOW2},
    {"show3", ShowMode::SHOW3}, {"show4", ShowMode::SHOW4}, {"show5", ShowMode::SHOW5},
    {"show6", ShowMode::SHOW6}, {"random", ShowMode::RANDOM},
};

const NamedValue COLOR_MACRO_NAMES[] = {
    {"off", ColorMacro::OFF},
    {"jump", ColorMacro::JUMP},
    {"fade1", ColorMacro::FADE1},
    {"fade2", ColorMacro::FADE2},
};

const NamedValue STROBE_NAMES[] = {
    {"off", Strobe::OFF},     {"slow", Strobe::SLOW}, {"medium", Strobe::MEDIUM},
    {"fast", Strobe::FAST},   {"sound", Strobe::SOUND},
};

const NamedValue UV_NAMES[] = {
    {"off", UV::OFF},
    {"on", UV::ON},
};

const NamedValue UV_CHASE_NAMES[] = {
    {"off", UVChase::OFF},
    {"slow", UVChase::SLOW},
    {"fast", UVChase::FAST},
    {"strobeslow", UVChase::STROBE_SLOW},
    {"strobefast", UVChase::STROBE_FAST},
};

const NamedValue LASER_NAMES[] = {
    {"off", Laser::OFF},
    {"red", Laser::RED},
    {"green", Laser::GREEN},
    {"redgreen", Laser::RED_GREEN},
    {"rgflicker", Laser::RG_FLICKER},
    {"grflicker", Laser::GR_FLICKER},
    {"rgsync", Laser::RG_FLICKER_SYNC},
    {"rgalt", Laser::RG_FLICKER_ALTERNATE},
};

const NamedValue LED_ROTATION_NAMES[] = {
    {"stop", LedRotation::STOP},
    {"cwslow", LedRotation::CW_SLOW},
    {"cwfast", LedRotation::CW_FAST},
    {"ccwslow", LedRotation::CCW_SLOW},
    {"ccwfast", LedRotation::CCW_FAST},
};

const NamedValue LASER_ROTATION_NAMES[] = {
    {"statica", LaserRotation::STATIC_A},
    {"staticb", LaserRotation::STATIC_B},
    {"chaseslow", LaserRotation::CHASE_SLOW},
    {"chasemedium", LaserRotation::CHASE_MEDIUM},
    {"chasefast", LaserRotation::CHASE_FAST},
};

void printHelp() {
  Serial.println(F("Commands (lowercase, space-separated):"));
  Serial.println(F("  c <channel> <value>       raw DMX write (channel 1-16, value 0-255)"));
  Serial.println(F("  show <name|0-255>         off show1..show6 random"));
  Serial.println(F("  color <name|0-255>        off jump fade1 fade2"));
  Serial.println(F("  strobe <name|0-255>       off slow medium fast sound"));
  Serial.println(F("  uv <name|0-255>           off on"));
  Serial.println(F("  uvchase <name|0-255>      off slow fast strobeslow strobefast"));
  Serial.println(F("  laser <name|0-255>        off red green redgreen rgflicker grflicker rgsync rgalt"));
  Serial.println(F("  laserstrobe <name|0-255>  off slow medium fast sound"));
  Serial.println(F("  ledrot <name|0-255>       stop cwslow cwfast ccwslow ccwfast"));
  Serial.println(F("  laserrot <name|0-255>     statica staticb chaseslow chasemedium chasefast"));
  Serial.println(F("  step <index>              apply showSequence[index]"));
  Serial.println(F("  blackout                  zero all channels"));
  Serial.println(F("  dump                      print current channel values"));
  Serial.println(F("  run                       resume trigger-driven show"));
  Serial.println(F("  help                      show this message"));
}

void dumpChannels(Fixture &fx) {
  uint16_t base = fx.baseAddress();
  for (uint8_t i = 0; i < FIXTURE_CHANNEL_COUNT; i++) {
    Serial.print(F("ch "));
    Serial.print(base + i);
    Serial.print(F(": "));
    Serial.println(DMXSerial.read(base + i));
  }
}

void handleLine(char *line, Fixture &fx) {
  char *cmd = strtok(line, " \t");
  if (cmd == nullptr) return;

  char *arg1 = strtok(nullptr, " \t");
  char *arg2 = strtok(nullptr, " \t");

  if (strcmp(cmd, "help") == 0) {
    printHelp();
  } else if (strcmp(cmd, "run") == 0) {
    overrideActive = false;
    Serial.println(F("resuming trigger-driven show"));
  } else if (strcmp(cmd, "blackout") == 0) {
    fx.blackout();
    overrideActive = true;
  } else if (strcmp(cmd, "dump") == 0) {
    dumpChannels(fx);
  } else if (strcmp(cmd, "step") == 0 && arg1 != nullptr) {
    int index = atoi(arg1);
    if (index >= 0 && (size_t)index < showSequenceLength) {
      applyShowStep(fx, showSequence[index]);
      overrideActive = true;
    } else {
      Serial.println(F("step index out of range"));
    }
  } else if (strcmp(cmd, "c") == 0 && arg1 != nullptr && arg2 != nullptr) {
    DMXSerial.write(atoi(arg1), (uint8_t)constrain(atoi(arg2), 0, 255));
    overrideActive = true;
  } else if (strcmp(cmd, "show") == 0 && arg1 != nullptr) {
    fx.setShowMode(resolve(arg1, SHOW_MODE_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "color") == 0 && arg1 != nullptr) {
    fx.setColorMacro(resolve(arg1, COLOR_MACRO_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "strobe") == 0 && arg1 != nullptr) {
    fx.setLedStrobe(resolve(arg1, STROBE_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "uv") == 0 && arg1 != nullptr) {
    fx.setUV(resolve(arg1, UV_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "uvchase") == 0 && arg1 != nullptr) {
    fx.setUVChase(resolve(arg1, UV_CHASE_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "laser") == 0 && arg1 != nullptr) {
    fx.setLasers(resolve(arg1, LASER_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "laserstrobe") == 0 && arg1 != nullptr) {
    fx.setLaserStrobe(resolve(arg1, STROBE_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "ledrot") == 0 && arg1 != nullptr) {
    fx.setLedRotation(resolve(arg1, LED_ROTATION_NAMES));
    overrideActive = true;
  } else if (strcmp(cmd, "laserrot") == 0 && arg1 != nullptr) {
    fx.setLaserRotation(resolve(arg1, LASER_ROTATION_NAMES));
    overrideActive = true;
  } else {
    Serial.println(F("unknown command, type 'help'"));
  }
}

} // namespace

namespace Console {

void begin(Fixture & /*fx*/) {
  if (!ENABLE_DMX_CONSOLE) return;
  Serial.begin(SERIAL_BAUD);
  Serial.println(F("FortuneTellerDMXRelay console - type 'help' for commands"));
}

bool poll(Fixture &fx) {
  if (!ENABLE_DMX_CONSOLE) return false;

  static char buffer[32];
  static size_t length = 0;

  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (length > 0) {
        buffer[length] = '\0';
        handleLine(buffer, fx);
        length = 0;
      }
    } else if (length < sizeof(buffer) - 1) {
      buffer[length++] = c;
    }
  }

  return overrideActive;
}

} // namespace Console
