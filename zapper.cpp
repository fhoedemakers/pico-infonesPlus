#include "zapper.h"
#include "pico/stdlib.h"
#include <stdio.h>

// Pico DVI pins (breadboard and PCB)
// D3 28
// D4 27
// WaveShare RP2040 DVI
// D3 14
// D4 15
// Adafruit Feather RP2040 DVI
// D3 11
// D4 12
// Pimoroni Pico DV Demo
// D3 ??
// D4 ??
#ifndef ZAPPER_D3
#define ZAPPER_D3 28      // light detection
#endif

#ifndef ZAPPER_D4
#define ZAPPER_D4 27      // trigger
#endif

bool zapperconnected = false;

#define ZAPPER_TRIGGER 0b10000
#define ZAPPER_LIGHTDETECT 0b01000
void initzapper(){
    // Initialize the zapper
#if ZAPPER_D3 != -1
    gpio_init(ZAPPER_D3);
    gpio_set_dir(ZAPPER_D3, GPIO_IN);
    gpio_pull_up(ZAPPER_D3);

    gpio_init(ZAPPER_D4);
    gpio_set_dir(ZAPPER_D4, GPIO_IN);
    gpio_pull_up(ZAPPER_D4);
#endif
    zapperconnected = false;
}
// https://www.nesdev.org/zapper_to_famicom.txt
// https://wiki.nesdev.com/w/index.php/Zapper
// D4 is the trigger, D3 is the light detection
int readzapperOLD(){
    // Read the zapper
    zapperconnected = false;
    int zapper = (gpio_get(ZAPPER_D4) == 0) ? 0 : ZAPPER_TRIGGER; 
    int lightgun = (gpio_get(ZAPPER_D3) == 0) ? ZAPPER_LIGHTDETECT : 0;
    //if (lightgun || zapper) printf("Lightgun: %d %d\n", zapper, lightgun);
    zapperconnected = (zapper || lightgun) ;
    return zapper | lightgun;
}

int readzapper(){
    // Read the zapper
    // When NES controller is attached, ZAPPER_D3 and ZAPPER_D4 are always high. (1)
    int zapperResult = 0;
#if ZAPPER_D3 != -1	
    int zapper = gpio_get(ZAPPER_D4);
    int lightgun = gpio_get(ZAPPER_D3);
    if ( zapper == 0 || lightgun == 0) {
       if (!zapperconnected) {
           zapperconnected = true;
           printf("Zapper connected\n");
       }
       
       zapper = (zapper == 0) ? 0 : ZAPPER_TRIGGER ;
       lightgun = (lightgun == 0) ? ZAPPER_LIGHTDETECT : 0;
       zapperResult = zapper | lightgun;
    }
    //printf("Zapper connected: %d, trigger: %d lightgun: %d\n", zapperconnected,  zapper, lightgun);
 #endif
    return zapperResult;
}