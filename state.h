#pragma once
#include "InfoNES_Types.h"
#define SAVESTATEDIR "/SAVESTATES"
// Maximum number of save state slots
#define MAXSAVESTATESLOTS 5
int InfoNES_SaveState(const char* path);
int InfoNES_LoadState(const char* path);