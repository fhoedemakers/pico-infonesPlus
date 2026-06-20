// Host-build shim for the Pico SDK's pico.h. The InfoNES core only uses the
// flash-placement macros, which are meaningless on a host build.
#pragma once
#include <stdint.h>

#ifndef __not_in_flash_func
#define __not_in_flash_func(f) f
#endif
#ifndef __time_critical_func
#define __time_critical_func(f) f
#endif
#ifndef __not_in_flash
#define __not_in_flash(group) /* host: no flash placement annotation */
#endif
#ifndef __in_flash
#define __in_flash(group) /* host */
#endif
#ifndef __scratch_x
#define __scratch_x(group) /* host */
#endif
#ifndef __scratch_y
#define __scratch_y(group) /* host */
#endif
