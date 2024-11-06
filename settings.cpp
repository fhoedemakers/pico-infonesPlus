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
    printf("fgcolor: %d\n", settings.fgcolor);
    printf("bgcolor: %d\n", settings.bgcolor);
    printf("\n");
}
void resetsettings()
{
    // Reset settings to default
    printf("Resetting settings\n");
    settings.screenMode = {};
    settings.firstVisibleRowINDEX = 0;
    settings.selectedRow = 0;
    settings.horzontalScrollIndex = 0;
    settings.fgcolor = DEFAULT_FGCOLOR;
    settings.bgcolor = DEFAULT_BGCOLOR;
    strcpy(settings.currentDir, "/");
}

void savesettings()
{
    // Save settings to file
    FIL fil;
    UINT bw;
    FRESULT fr;
    printf("Saving settings to %s\n", SETTINGSFILE);
    fr = f_open(&fil, SETTINGSFILE, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr == FR_OK)
    {
        fr = f_write(&fil, &settings, sizeof(settings), &bw);
        if (fr)
        {
            printf("Error writing %s: %d\n", SETTINGSFILE, fr);
        }
        else
        {
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
    FIL fil;
    UINT br;
    FRESULT fr;
    DIR dir;
    FILINFO fno;
    // Load settings from file
    printf("Loading settings\n");
    // determine size of settings file
    fr = f_stat(SETTINGSFILE, &fno);
    if (fr == FR_OK)
    {
        printf("Size of %s: %lu bytes\n", SETTINGSFILE, fno.fsize);
        if (fno.fsize != sizeof(settings))
        {
            printf("Size of %s is not %d bytes, resetting settings\n", SETTINGSFILE, sizeof(settings));
            resetsettings();
        }
        else
        {
            fr = f_open(&fil, SETTINGSFILE, FA_READ);
            if (fr == FR_OK)
            {
                fr = f_read(&fil, &settings, sizeof(settings), &br);
                if (fr)
                {
                    printf("Error reading %s: %d\n", SETTINGSFILE, fr);
                    resetsettings();
                }
                else
                {
                    // If file is corrupt, reset settings to default
                    if (br != sizeof(settings))
                    {
                        printf("File %s is corrupt, expected %d bytes, read %d\n", SETTINGSFILE, sizeof(settings), br);
                        resetsettings();
                    }
                    else
                    {
                        printf("Read %d bytes from %s\n", br, SETTINGSFILE);
                    }
                }
                f_close(&fil);
            }
            else
            {
                // If file does not exist, reset settings to default
                if (fr == FR_NO_FILE)
                {
                    printf("File %s does not exist\n", SETTINGSFILE);                  
                }
                else
                {
                    printf("Error opening %s: %d\n", SETTINGSFILE, fr);
                }
                resetsettings();
            }

            // if settings.currentDir is no valid directory, reset settings to default
            if (f_opendir(&dir, settings.currentDir) != FR_OK)
            {
                printf("Directory %s does not exist\n", settings.currentDir);
                resetsettings();
            }
        }
    }
    else
    {
        printf("Settings file not found %s: %d\n", SETTINGSFILE, fr);
        resetsettings();
    }
    printsettings();
}
