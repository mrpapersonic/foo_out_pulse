/* winelib wrapper for pulseaudio functions.
 *
 * written by Paper, 2025-06-27, for foo_out_pulse
 *
 * `winegcc -o pulseaudio-wrapper.dll pulseaudio-wrapper.c -shared -lpulse` */

#include <pulse/pulseaudio.h>
#include <windows.h>

#include "pulseaudio-wrapper.h"

#define DECL_FUNC_EX(type, name, paramswtype, params, RETURN /* return, or nothing at all */) \
	__attribute__((ms_abi)) WINAPI type WIN_##name paramswtype \
	{ \
		RETURN name params; \
	}

#define DECL_FUNC(type, name, paramswtype, params) \
	DECL_FUNC_EX(type, name, paramswtype, params, return)
#define DECL_FUNC_VOID(name, paramswtype, params) \
	DECL_FUNC_EX(void, name, paramswtype, params, /* nothing */)

/* ok, this is confusing, but
 * all pulseaudio callbacks function under the normal C callback preconditions,
 * so (in general), there is a `void *` user data pointer that can be passed.
 * thus, we can "generalize" this by making one big callback handler.
 *
 * YES, this results in a memory leak. I don't care :) it's 16 bytes.
 *
 * TODO: callback data and params are generally linked to a type, not a function,
 * and as such we should probably be defining cb data via type and not funcs. */
#define DECL_FUNC_CB_EX(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams, RETURN) \
	struct name##_cb_data { \
		cbtype (__attribute__((ms_abi)) WINAPI *cb) cbparamswtype; \
		void *userdata; \
	}; \
	\
	/* FIXME: return type essentially hardcoded to NULL */ \
	cbtype name##_cb cbparamswtype \
	{ \
		struct name##_cb_data *cb_data = userdata; \
	\
		/* ok */ \
		if (cb_data->cb) \
			cb_data->cb cbparams; \
	} \
	\
	__attribute__((ms_abi)) WINAPI type WIN_##name paramswtype \
	{ \
		struct name##_cb_data *cb_data; \
	\
		/* FIXME: we shouldn't call back at all if the func is NULL */ \
		cb_data = malloc(sizeof(*cb_data)); \
		/* hmm, assert here? */ \
	\
		cb_data->cb = cb; \
		cb_data->userdata = (void *)userdata; \
	\
		RETURN name params; \
	}

#define DECL_FUNC_CB(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
	DECL_FUNC_CB_EX(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams, return)

#define DECL_FUNC_CB_VOID(name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
	DECL_FUNC_CB_EX(void, name, paramswtype, params, cbtype, cbparamswtype, cbparams, /* nothing */)

#include "pulseaudio-wrapper.h"

/* dummy func */
__attribute__((ms_abi)) WINAPI void foo_out_pulse_winelib_dll(void) { }
