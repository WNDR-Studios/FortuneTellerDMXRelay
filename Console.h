// Console.h
// Interactive serial console for live DMX exploration. Lets you poke at the
// Stinger II's channels over the Serial Monitor while connected to real
// hardware, using the same Fixture setters the show uses. See README.md for
// the full command reference.
#pragma once

#include "Fixture.h"
#include "Show.h"

extern const ShowStep* activeShow;
extern size_t activeShowLength;

namespace Console {

// Initializes the serial console if ENABLE_DMX_CONSOLE is true (Config.h).
// No-op otherwise.
void begin(Fixture &fx);

// Reads and dispatches any pending console input (non-blocking). Returns true
// if a console command has put the fixture into a manual "override" state, in
// which case the caller should skip the trigger-driven state machine for this
// iteration. Returns false immediately if the console is disabled.
bool poll(Fixture &fx);

} // namespace Console
