// Fixture.cpp
#include "Fixture.h"

void Fixture::blackout() {
  for (uint8_t i = 0; i < FIXTURE_CHANNEL_COUNT; i++) {
    DMXSerial.write(_base + i, 0);
  }
}
