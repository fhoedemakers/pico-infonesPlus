#include "settings.h"
#include <stdio.h>
#include <string.h>

struct settings settings;

void printsettings()
{
    printf("Settings:\n");
    printf("ScreenMode: %d\n", (int)settings.screenMode);
    printf("firstVisibleRowINDEX: %d\n", settings.firstVisibleRowINDEX);
    printf("selectedRow: %d\n", settings.selectedRow);
    printf("horzontalScrollIndex: %d\n", settings.horzontalScrollIndex);
    printf("currentDir: %s\n", settings.currentDir);
    printf("\n");
}

void savesettings()
{
    // Save settings to file
    FIL fil;
    UINT bw;
    FRESULT fr;
    printf("Saving settings to %s\n",  SETTINGSFILE);
    fr = f_open(&fil, SETTINGSFILE, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr == FR_OK)
    {
        fr = f_write(&fil, &settings, sizeof(settings), &bw);
        if (fr)
        {
            printf("Error writing %s: %d\n", SETTINGSFILE, fr);
        } else {
            printf("Wrote %d bytes to %s\n", bw, SETTINGSFILE);
        }
         f_close(&fil);
    }
    else
    {
        printf("Error opening %s: %d\n", SETTINGSFILE, fr);
    }
    printsettings();
}

void loadsettings()
{
    // Load settings from file
    printf("Loading settings\n");

    resetsettings();
    printsettings();
}

void resetsettings()
{
    // Reset settings to default
    printf("Resetting settings\n");
    settings.screenMode = {};
    settings.firstVisibleRowINDEX = 0;
    settings.selectedRow = 0;
    settings.horzontalScrollIndex = 0;
    strcpy(settings.currentDir, "/");
}
