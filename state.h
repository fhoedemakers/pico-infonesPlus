#pragma once
#include "InfoNES_Types.h"

int Emulator_SaveState(const char* path);
int Emulator_LoadState(const char* path);