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

void setup() {
  // Relay NO contact connects TRIGGER_PIN to GND when energized.
  // Internal pull-up holds the pin HIGH at rest; LOW = trigger active.
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  DMXSerial.init(DMXController, DMX_MODE_PIN);
  DMXSerial.maxChannel(DMX_MAX_CHANNEL);
  fixture.blackout();

  Console::begin(fixture);
}

void loop() {
  if (Console::poll(fixture)) {
    return; // console override active; skip the trigger-driven state machine
  }

  bool triggerActive = digitalRead(TRIGGER_PIN) == LOW;
  unsigned long now = millis();

  switch (state) {
    case State::IDLE:
      if (triggerActive) {
        stepIndex = 0;
        stepStartMs = now;
        applyShowStep(fixture, showSequence[stepIndex]);
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
      if (now - stepStartMs >= showSequence[stepIndex].durationMs) {
        stepIndex = (stepIndex + 1) % showSequenceLength; // loop while triggered
        stepStartMs = now;
        applyShowStep(fixture, showSequence[stepIndex]);
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
