#include "settings.h"
#include <stdio.h>
#include <string.h>

struct settings settings;

void savesettings() {
    // Save settings to file
    printf("Saving settings\n");
}

void loadsettings() {
    // Load settings from file
    printf("Loading settings\n");
    settings.screenMode = {};
    settings.firstVisibleRowINDEX = 0;
    settings.selectedRow = 0;
    settings.horzontalScrollIndex = 0;
    strcpy(settings.currentDir, "/");
}