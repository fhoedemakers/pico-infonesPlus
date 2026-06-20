// Host-build shim. The real settings.h is part of the device build's
// settings/menu system. Provide just enough struct depth that the
// FDS_AutoInsertEnabled macro in InfoNES_FDS.h compiles and reads sensibly.
#pragma once

struct HostSettingsFlags {
    int autoSwapFDS;
    int autoInsertDiskA;
};

struct HostSettings {
    HostSettingsFlags flags;
};

extern HostSettings settings;
