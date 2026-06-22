// FortuneTellerDMXRelay.ino
//
// Trigger-driven DMX laser show controller for the fortune teller cabinet.
// See README.md for wiring, build instructions, and how to design the show.
//
// State machine:
//   IDLE     - trigger inactive. DMX buffer holds all-zero; DMXSerial keeps refreshing it.
//   ACTIVE   - trigger active. Steps through showSequence (Show.h), looping for as
//              long as the trigger stays active.
//   BLACKOUT - trigger went inactive. Buffer is zeroed immediately and held for
//              BLACKOUT_HOLD_MS before returning to IDLE.
#include <DMXSerial.h>
#include "Config.h"
#include "Fixture.h"
#include "Show.h"
#include "Console.h"

enum class State : uint8_t { IDLE, ACTIVE, BLACKOUT };

Fixture fixture(FIXTURE_ADDRESS);
State state = State::IDLE;
size_t stepIndex = 0;
unsigned long stepStartMs = 0;
unsigned long blackoutStartMs = 0;

const ShowStep* activeShow = allShows[0];
size_t activeShowLength = allShowLengths[0];

// Shuffle pool — tracks which shows haven't been played yet this cycle.
// poolIndex starts at NUM_SHOWS so the first pick triggers a fresh shuffle.
uint8_t showPool[NUM_SHOWS];
uint8_t poolIndex = NUM_SHOWS;

void shufflePool() {
  for (uint8_t i = 0; i < NUM_SHOWS; i++) showPool[i] = i;
  for (uint8_t i = NUM_SHOWS - 1; i > 0; i--) {
    uint8_t j = (uint8_t)random(i + 1);
    uint8_t tmp = showPool[i];
    showPool[i] = showPool[j];
    showPool[j] = tmp;
  }
  poolIndex = 0;
}

uint8_t pickNextShow() {
  if (poolIndex >= NUM_SHOWS) shufflePool();
  return showPool[poolIndex++];
}

bool lastRawTrigger = false;
bool debouncedTrigger = false;
unsigned long triggerLastChangeMs = 0;

void setup() {
  // Relay NO contact connects TRIGGER_PIN to GND when energized.
  // Internal pull-up holds the pin HIGH at rest; LOW = trigger active.
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  randomSeed(analogRead(A0));

  DMXSerial.init(DMXController, DMX_MODE_PIN);
  DMXSerial.maxChannel(DMX_MAX_CHANNEL);
  fixture.blackout();

  Console::begin(fixture);
}

void loop() {
  if (Console::poll(fixture)) {
    return; // console override active; skip the trigger-driven state machine
  }

  unsigned long now = millis();
  bool rawTrigger = (digitalRead(TRIGGER_PIN) == LOW);
  if (rawTrigger != lastRawTrigger) {
    lastRawTrigger = rawTrigger;
    triggerLastChangeMs = now;
  }
  if ((now - triggerLastChangeMs) >= TRIGGER_DEBOUNCE_MS) {
    debouncedTrigger = rawTrigger;
  }
  bool triggerActive = debouncedTrigger;

  switch (state) {
    case State::IDLE:
      if (triggerActive) {
        uint8_t pick = pickNextShow();
        activeShow = allShows[pick];
        activeShowLength = allShowLengths[pick];
        stepIndex = 0;
        stepStartMs = now;
        applyShowStep(fixture, activeShow[stepIndex]);
        state = State::ACTIVE;
      }
      break;

    case State::ACTIVE:
      if (!triggerActive) {
        fixture.blackout();
        blackoutStartMs = now;
        state = State::BLACKOUT;
        break;
      }
      if (now - stepStartMs >= activeShow[stepIndex].durationMs) {
        stepIndex = (stepIndex + 1) % activeShowLength;
        stepStartMs = now;
        applyShowStep(fixture, activeShow[stepIndex]);
      }
      break;

    case State::BLACKOUT:
      // Buffer is already all-zero; DMXSerial's background transmitter keeps
      // re-sending it. Just hold here before returning to idle.
      if (now - blackoutStartMs >= BLACKOUT_HOLD_MS) {
        state = State::IDLE;
      }
      break;
  }
}
