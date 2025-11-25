#pragma once
#include "InfoNES_Types.h"
#define SAVESTATEDIR "/SAVESTATES"
// Maximum number of save state slots
#define MAXSAVESTATESLOTS 5
int Emulator_SaveState(const char* path);
int Emulator_LoadState(const char* path);