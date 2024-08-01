#include "settings.h"
#include <stdio.h>
#include <string.h>

struct settings settings;


void printsettings() {
    printf("Settings:\n");
    printf("ScreenMode: %d\n", (int)settings.screenMode);
    printf("firstVisibleRowINDEX: %d\n", settings.firstVisibleRowINDEX);
    printf("selectedRow: %d\n", settings.selectedRow);
    printf("horzontalScrollIndex: %d\n", settings.horzontalScrollIndex);
    printf("currentDir: %s\n", settings.currentDir);
    printf("\n");
}

void savesettings() {
    // Save settings to file
    printf("Saving settings\n");
    printsettings();
}

void loadsettings() {
    // Load settings from file
    printf("Loading settings\n");
    
    resetsettings();
    printsettings();
}

void resetsettings() {
    // Reset settings to default
    printf("Resetting settings\n");
    settings.screenMode = {};
    settings.firstVisibleRowINDEX = 0;
    settings.selectedRow = 0;
    settings.horzontalScrollIndex = 0;
    strcpy(settings.currentDir, "/");
}   

