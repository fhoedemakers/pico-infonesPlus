#pragma once
#include <stdint.h>
// Visibility-controlled menu settings for emulator configuration
// Each index corresponds to an option line when settings menu is opened via SELECT.
// Value 1 in g_option_visibility means the option is shown, 0 means hidden for current emulator.

enum MenuSettingsIndex {
    MOPT_SCREENMODE = 0,
    MOPT_SCANLINES,
    MOPT_FPS_OVERLAY,
    MOPT_AUDIO_ENABLE,
    MOPT_EXTERNAL_AUDIO,
    MOPT_FONT_COLOR,
    MOPT_FONT_BACK_COLOR,
    MOPT_FRUITJAM_VUMETER,
    MOPT_DMG_PALETTE,
    MOPT_BORDER_MODE,
    MOPT_FRAMESKIP,
    MOPT_FRUITJAM_INTERNAL_SPEAKER,
    MOPT_COUNT
};

extern const uint8_t g_settings_visibility[MOPT_COUNT];

// Available screen modes for selection in settings menu
extern const uint8_t g_available_screen_modes[];
