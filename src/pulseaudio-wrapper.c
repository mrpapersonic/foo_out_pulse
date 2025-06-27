/* winelib wrapper for pulseaudio functions.
 *
 * written by Paper, 2025-06-27, for foo_out_pulse
 *
 * `winegcc -o pulseaudio-wrapper.dll pulseaudio-wrapper.c -shared -lpulse` */

#include <pulse/pulseaudio.h>
#include <windows.h>

#define DECL_FUNC(type, name, paramswtype, params) \
	__attribute__((ms_abi)) WINAPI type WIN_##name paramswtype \
	{ \
		return name params; \
	}
#define DECL_FUNC_VOID(name, paramswtype, params) \
	__attribute__((ms_abi)) WINAPI void WIN_##name paramswtype \
	{ \
		name params; \
	}
#include "pulseaudio-wrapper.h"

/* tell foo_out_pulse we are using winelib */
extern int foo_out_pulse_winelib_dll;

/* wuh */
int foo_out_pulse_winelib_dll = 1;
