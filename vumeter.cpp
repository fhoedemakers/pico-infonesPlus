#include <stdio.h>
#include <cstdlib>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "ws2812.pio.h" // Generated PIO program for WS2812

#define LED_PIN 32    // Your data pin (GPIO32)
#define LED_COUNT 5   // Number of LEDs in strip
#define IS_RGBW false // Set true if you use RGBW pixels
#define SAMPLES_PER_SCANLINE 4
#define SCANLINES 240
#define NEOPIXELPIO pio1

static int16_t frame_buffer[SCANLINES * SAMPLES_PER_SCANLINE];
static int sample_index = 0;
static PIO pio;
static uint sm;
static uint offset;

// Send one pixel color to the PIO state machine
static inline void put_pixel(uint32_t pixel_grb)
{
    //printf("Pixel SM: %d GRB: %06X\n", sm, pixel_grb);
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

// Pack RGB into 24-bit GRB value
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)g << 16) |
           ((uint32_t)r << 8) |
           (uint32_t)b;
}
static float dynamic_max = 2000.0f; // start guess
void process_audio_frame(int16_t *buf, int count)
{
    long sum = 0;
    for (int i = 0; i < count; i++)
    {
        sum += abs(buf[i]);
    }
    int avg = sum / count; // average amplitude

    if (avg > dynamic_max)
    {
        dynamic_max = avg; // track peaks
    }
    else
    {
        dynamic_max *= 0.995f; // decay slowly
    }

    float norm = avg / dynamic_max;
    if (norm > 1.0f)
        norm = 1.0f;

    //printf("Avg amplitude: %d, max amplitude: %f, Normalized: %.3f\n", avg, dynamic_max, norm);
    int lit = (int)(norm * LED_COUNT);
    if (lit > LED_COUNT)
        lit = LED_COUNT;

    // update LEDs
    for (int i = 0; i < LED_COUNT; i++)
    {
        if (LED_COUNT - i < lit)
        {
            put_pixel(urgb_u32(0, 255, 0)); // green
            //printf("LED %d: ON\n", i);
        }
        else
        {
            put_pixel(urgb_u32(0, 0, 0)); // off
            //printf("LED %d: OFF\n", i);
        }
       
    }
}

void reset_leds() {
    for (int i = 0; i < LED_COUNT; i++)
    {
        put_pixel(urgb_u32(0, 0, 0)); // off
    }
}
void initNeoPixel()
{
    
    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, LED_PIN, 1, true);
    hard_assert(success);

    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, IS_RGBW);
}

void vu_sample_callback(int16_t sample)
{

    frame_buffer[sample_index++] = sample;
    // When we have enough samples for a frame, process them

    if (sample_index >= SCANLINES * SAMPLES_PER_SCANLINE)
    {
        process_audio_frame(frame_buffer, sample_index);
        sample_index = 0;
    }
}
