/* winelib wrapper for pulseaudio functions.
 *
 * written by Paper, 2025-06-27, for foo_out_pulse
 *
 * `winegcc -o pulseaudio-wrapper.dll pulseaudio-wrapper.c -shared -lpulse` */

#include <stdbool.h> /* pulse */

#include <pulse/pulseaudio.h>
#include <windows.h>

#include <poll.h>

#include "pulseaudio-wrapper.h"

#define DECL_FUNC_EX(type, name, paramswtype, params, RETURN /* return, or nothing at all */) \
	__attribute__((ms_abi)) WINAPI type WIN_##name paramswtype \
	{ \
		RETURN name params; \
	}

#define DECL_FUNC(type, name, paramswtype, params) DECL_FUNC_EX(type, name, paramswtype, params, return)
#define DECL_FUNC_VOID(name, paramswtype, params)  DECL_FUNC_EX(void, name, paramswtype, params, /* nothing */)

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
		cbtype(__attribute__((ms_abi)) WINAPI *cb) cbparamswtype; \
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
		cbtype (*cbs) cbparamswtype; \
\
		if (cb) {\
			cbs = name##_cb; \
			cb_data = malloc(sizeof(*cb_data)); \
			/* hmm, assert here? */ \
\
			cb_data->cb = cb; \
			cb_data->userdata = (void *)userdata; \
		} else { \
			cbs = NULL; \
			cb_data = NULL; \
		} \
\
		RETURN name params; \
	}

#define DECL_FUNC_CB(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
	DECL_FUNC_CB_EX(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams, return)

#define DECL_FUNC_CB_VOID(name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
	DECL_FUNC_CB_EX(void, name, paramswtype, params, cbtype, cbparamswtype, cbparams, /* nothing */)

#include "pulseaudio-wrapper.h"

/* dummy func */
__attribute__((ms_abi)) WINAPI void foo_out_pulse_winelib_dll(void)
{
}

/* ------------------------------------------------------------------------ */
/* custom implementation of pulseaudio-threaded-mainloop
 *
 * RATIONALE: if we use the pulseaudio routines, wine has no damn idea
 * where we're calling from, resulting in a big crash (SAD!)
 *
 * thus, we need to reimplement these functions using windows threads. */

typedef struct WIN_pa_threaded_mainloop WIN_pa_threaded_mainloop;

/***
  This file is part of PulseAudio.

  Copyright 2006 Lennart Poettering
  Copyright 2006 Pierre Ossman <ossman@cendio.se> for Cendio AB

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#include <pulse/mainloop.h>
#include <pulse/xmalloc.h>

/* ------------------------------------------------------------------------ */
/* pa_atomic */

typedef struct WIN_pa_atomic {
	volatile LONG x;
} WIN_pa_atomic;

static inline __attribute__((__always_inline__))
int WIN_pa_atomic_load(WIN_pa_atomic *a)
{
	/* i think this is correct? */
	return a->x;
}

static inline __attribute__((__always_inline__))
void WIN_pa_atomic_store(WIN_pa_atomic *a, int i)
{
	InterlockedExchange(&a->x, i);
}

/* ------------------------------------------------------------------------ */

#include <stdio.h>

typedef void (*pa_thread_func_t)(void *userdata);

typedef struct WIN_pa_thread {
	HANDLE thread;
	pa_thread_func_t thread_func;

	void *userdata;
} WIN_pa_thread;

__attribute__((ms_abi)) __attribute__((__force_align_arg_pointer__)) static DWORD WINAPI
internal_thread_func(LPVOID param)
{
	WIN_pa_thread *t = param;

	t->thread_func(t->userdata);

	return 0;
}

static inline __attribute__((__always_inline__))
WIN_pa_thread *WIN_pa_thread_new(const char *name, pa_thread_func_t thread_func, void *userdata)
{
	WIN_pa_thread *t;
	DWORD thread_id;

	t = calloc(1, sizeof(*t));
	t->thread_func = thread_func;
	t->userdata = userdata;

	t->thread = CreateThread(NULL, 0, internal_thread_func, t, 0, &thread_id);

	if (!t->thread) {
		free(t);
		return NULL;
	}

	return t;
}

static inline __attribute__((__always_inline__))
int WIN_pa_thread_is_running(WIN_pa_thread *t)
{
	DWORD code;

	if (!GetExitCodeThread(t->thread, &code))
		return 0;

	return (code == STILL_ACTIVE);
}

static inline __attribute__((__always_inline__))
int WIN_pa_thread_join(WIN_pa_thread *t);

static inline __attribute__((__always_inline__))
void WIN_pa_thread_free(WIN_pa_thread *t)
{
	WIN_pa_thread_join(t);
	CloseHandle(t->thread);
	free(t);
}

static inline __attribute__((__always_inline__))
void WIN_pa_thread_free_nojoin(WIN_pa_thread *t)
{
	CloseHandle(t->thread);
	free(t);
}

static inline __attribute__((__always_inline__))
int WIN_pa_thread_join(WIN_pa_thread *t)
{
	if (WaitForSingleObject(t->thread, INFINITE) == WAIT_FAILED)
		return -1;

	return 0;
}

static inline __attribute__((__always_inline__))
uint32_t WIN_pa_thread_id(WIN_pa_thread *t)
{
	return GetThreadId(t->thread);
}

static inline __attribute__((__always_inline__))
uint32_t WIN_pa_thread_current_id(WIN_pa_thread *t)
{
	return GetCurrentThreadId();
}

static inline __attribute__((__always_inline__))
int WIN_pa_thread_current(WIN_pa_thread *t)
{
	return t && (GetThreadId(t->thread) == GetCurrentThreadId());
}

static inline __attribute__((__always_inline__))
void *WIN_pa_thread_get_data(WIN_pa_thread *t)
{
	return t->userdata;
}

static inline __attribute__((__always_inline__))
void WIN_pa_thread_set_data(WIN_pa_thread *t, void *userdata)
{
	t->userdata = userdata;
}

static inline __attribute__((__always_inline__))
void WIN_pa_thread_set_name(WIN_pa_thread *t, const char *name)
{
	/* Not implemented */
}

static inline __attribute__((__always_inline__))
const char *WIN_pa_thread_get_name(WIN_pa_thread *t)
{
	/* Not implemented */
	return NULL;
}

static inline __attribute__((__always_inline__))
void WIN_pa_thread_yield(void)
{
	Sleep(0);
}

/* ------------------------------------------------------------------------ */

typedef struct WIN_pa_mutex {
	CRITICAL_SECTION mutex;
} WIN_pa_mutex;

typedef struct WIN_pa_cond {
	CONDITION_VARIABLE cond;
} WIN_pa_cond;

static inline __attribute__((__always_inline__))
WIN_pa_mutex *WIN_pa_mutex_new(bool recursive, bool inherit_priority)
{
	WIN_pa_mutex *m;

	m = calloc(1, sizeof(*m));

	InitializeCriticalSection(&m->mutex);

	return m;
}

static inline __attribute__((__always_inline__))
void WIN_pa_mutex_free(WIN_pa_mutex *m)
{
	DeleteCriticalSection(&m->mutex);
	free(m);
}

static inline __attribute__((__always_inline__))
void WIN_pa_mutex_lock(WIN_pa_mutex *m)
{
	EnterCriticalSection(&m->mutex);
}

static inline __attribute__((__always_inline__))
void WIN_pa_mutex_unlock(WIN_pa_mutex *m)
{
	LeaveCriticalSection(&m->mutex);
}

static inline __attribute__((__always_inline__))
WIN_pa_cond *WIN_pa_cond_new(void)
{
	WIN_pa_cond *c;

	c = calloc(1, sizeof(*c));
	InitializeConditionVariable(&c->cond);

	return c;
}

static inline __attribute__((__always_inline__))
void WIN_pa_cond_free(WIN_pa_cond *c)
{
	free(c);
}

static inline __attribute__((__always_inline__))
void WIN_pa_cond_signal(WIN_pa_cond *c, int broadcast)
{
	if (broadcast) {
		WakeAllConditionVariable(&c->cond);
	} else {
		WakeConditionVariable(&c->cond);
	}
}

static inline __attribute__((__always_inline__))
int WIN_pa_cond_wait(WIN_pa_cond *c, WIN_pa_mutex *m)
{
	HANDLE event;

	SleepConditionVariableCS(&c->cond, &m->mutex, INFINITE);

	return 0;
}

/* ------------------------------------------------------------------------ */

struct WIN_pa_threaded_mainloop {
	pa_mainloop *real_mainloop;
	volatile int n_waiting, n_waiting_for_accept;
	WIN_pa_atomic in_once_unlocked;

	WIN_pa_thread *thread;
	WIN_pa_mutex *mutex;
	WIN_pa_cond *cond, *accept_cond;

	char *name;
};

static inline int in_worker(WIN_pa_threaded_mainloop *m)
{
	return WIN_pa_thread_current(m->thread);
}

static int poll_func(struct pollfd *ufds, unsigned long nfds, int timeout, void *userdata)
{
	WIN_pa_mutex *mutex = userdata;
	int r;

	/* Before entering poll() we unlock the mutex, so that
	 * avahi_simple_poll_quit() can succeed from another thread. */

	/* wait, avahi? isn't that for mDNS? WTF?
	 * no wonder pulseaudio is dying, yeesh */

	WIN_pa_mutex_unlock(mutex);
	r = poll(ufds, nfds, timeout);
	WIN_pa_mutex_lock(mutex);

	return r;
}

static void thread(void *userdata)
{
	WIN_pa_threaded_mainloop *m = userdata;

	WIN_pa_mutex_lock(m->mutex);

	(void)pa_mainloop_run(m->real_mainloop, NULL);

	WIN_pa_mutex_unlock(m->mutex);
}

__attribute__((ms_abi)) WINAPI WIN_pa_threaded_mainloop *WIN_pa_threaded_mainloop_new(void)
{
	WIN_pa_threaded_mainloop *m;

	m = calloc(1, sizeof(*m));

	m->real_mainloop = pa_mainloop_new();
	if (!m->real_mainloop) {
		free(m);
		return NULL;
	}

	m->mutex = WIN_pa_mutex_new(true, true);
	m->cond = WIN_pa_cond_new();
	m->accept_cond = WIN_pa_cond_new();
	if (!m->mutex || !m->cond || !m->accept_cond) {
		free(m);
		return NULL;
	}

	pa_mainloop_set_poll_func(m->real_mainloop, poll_func, m->mutex);

	return m;
}

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_stop(WIN_pa_threaded_mainloop *m);

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_free(WIN_pa_threaded_mainloop *m)
{
	/* Make sure that this function is not called from the helper thread */
	WIN_pa_threaded_mainloop_stop(m);

	if (m->thread)
		WIN_pa_thread_free(m->thread);

	pa_mainloop_free(m->real_mainloop);

	WIN_pa_mutex_free(m->mutex);
	WIN_pa_cond_free(m->cond);
	WIN_pa_cond_free(m->accept_cond);

	free(m->name);
	free(m);
}

__attribute__((ms_abi)) WINAPI int WIN_pa_threaded_mainloop_start(WIN_pa_threaded_mainloop *m)
{
	m->thread = WIN_pa_thread_new(m->name ? m->name : "threaded-ml", thread, m);

	if (!m->thread)
		return -1;

	return 0;
}

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_stop(WIN_pa_threaded_mainloop *m)
{
	if (!m->thread || !WIN_pa_thread_is_running(m->thread))
		return;

	WIN_pa_mutex_lock(m->mutex);
	pa_mainloop_quit(m->real_mainloop, 0);
	WIN_pa_mutex_unlock(m->mutex);

	WIN_pa_thread_join(m->thread);
}

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_lock(WIN_pa_threaded_mainloop *m)
{
	WIN_pa_mutex_lock(m->mutex);
}

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_unlock(WIN_pa_threaded_mainloop *m)
{
	WIN_pa_mutex_unlock(m->mutex);
}

/* Called with the lock taken */
__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_signal(WIN_pa_threaded_mainloop *m, int wait_for_accept)
{
	WIN_pa_cond_signal(m->cond, 1);

	if (wait_for_accept) {
		m->n_waiting_for_accept++;

		while (m->n_waiting_for_accept > 0)
			WIN_pa_cond_wait(m->accept_cond, m->mutex);
	}
}

/* Called with the lock taken */
__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_wait(WIN_pa_threaded_mainloop *m)
{
	m->n_waiting++;

	WIN_pa_cond_wait(m->cond, m->mutex);

	m->n_waiting--;
}

/* Called with the lock taken */
__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_accept(WIN_pa_threaded_mainloop *m)
{
	m->n_waiting_for_accept--;

	WIN_pa_cond_signal(m->accept_cond, 0);
}

__attribute__((ms_abi)) WINAPI int WIN_pa_threaded_mainloop_get_retval(const WIN_pa_threaded_mainloop *m)
{
	return pa_mainloop_get_retval(m->real_mainloop);
}

__attribute__((ms_abi)) WINAPI pa_mainloop_api *WIN_pa_threaded_mainloop_get_api(WIN_pa_threaded_mainloop *m)
{
	return pa_mainloop_get_api(m->real_mainloop);
}

__attribute__((ms_abi)) WINAPI int WIN_pa_threaded_mainloop_in_thread(WIN_pa_threaded_mainloop *m)
{
	return WIN_pa_thread_current(m->thread);
}

__attribute__((ms_abi)) WINAPI void WIN_pa_threaded_mainloop_set_name(WIN_pa_threaded_mainloop *m, const char *name)
{
	m->name = strdup(name);

	if (m->thread)
		WIN_pa_thread_set_name(m->thread, m->name);
}
