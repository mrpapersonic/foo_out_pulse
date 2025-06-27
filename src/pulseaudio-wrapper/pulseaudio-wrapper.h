/* pulseaudio wrapper crap.
 * note: the lack of header guards *is* intentional here.
 * the intent is that someone can get the interface simply by including
 * this file, while also being able to implement everything through macros,
 * ultimately reducing the amount of code needed to implement it...
 *
 * written by Paper, 2025-06-27, for foo_out_pulse */

#ifndef DECL_FUNC
# define DECL_FUNC(type, name, paramswtype, params) \
	WINAPI type WIN_##name paramswtype;
#endif

#ifndef DECL_FUNC_VOID
# define DECL_FUNC_VOID(name, paramswtype, params) \
	WINAPI void WIN_##name paramswtype;
#endif

#ifndef DECL_FUNC_CB
# define DECL_FUNC_CB(type, name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
	DECL_FUNC(type, name, paramswtype, params)
#endif

#ifndef DECL_FUNC_CB_VOID
# define DECL_FUNC_CB_VOID(name, paramswtype, params, cbtype, cbparamswtype, cbparams) \
    DECL_FUNC_VOID(name, paramswtype, params)
#endif

/*

#!/usr/bin/env python3
# python script that MOSTLY handles this;
# the only exception is functions with a (void) param (e.g. no params),
# which have to be made manually.
# additionally, there are no consideration about difference in type sizes
# (e.g. long is 64-bit on linux x86_64 while its 32-bit on win32 x86_64)
# because the functions we use simply don't have that problem.

#!/usr/bin/env python3

# this constant holds the function definition(s), grabbed from the doxygen page
# and separated with newlines.
strx = """void 	pa_stream_unref (pa_stream *s)"""

# ack!
LEGAL_IDENTIFIER_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_"

# whatever
def strpbrk_thing(s, charset):
    for i in range(len(s) - 1, -1, -1):
        if s[i] not in charset:
            return (s[:i+1], s[i+1:])
    return None

def split_c_name_and_type(string):
    y = strpbrk_thing(string, LEGAL_IDENTIFIER_CHARS)
    return (y[0].strip(), y[1].strip())

#return: (func return type, func name, (arg1type, arg1name), (arg2type, arg2name), ...)
def split_c_func_decl(string):
    x = string.split('(')
    x[0] = x[0].strip()
    x[1] = x[1].strip().rstrip(');').strip()
    args = x[1].split(', ')
    y = split_c_name_and_type(x[0])
    return y + tuple([split_c_name_and_type(arg.strip()) for arg in args])

def print_params_ex(x, print_func):
    # UGH
    if len(x) == 0:
        return

    for i in x[:-1]:
        print_func(i)
        print(", ", end="")
    print_func(x[-1])

def print_params_with_type(x):
    def c(i):
        print("%s %s" % (i[0], i[1]), end="")
    print_params_ex(x, c)

def print_params(x):
    def c(i):
        print("%s" % (i[1]), end="")
    print_params_ex(x, c)

for i in strx.split('\n'):
    func = split_c_func_decl(i)
    if func[0] == "void":
        print("DECL_FUNC_VOID(", end="")
    else:
        print("DECL_FUNC(%s, " % func[0], end="")
    print("%s, (" % (func[1]), end="")
    print_params_with_type(func[2:])
    print("), (", end="")
    print_params(func[2:])
    print("))")

*/

typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_free_cb_t)(void *);
typedef void(__attribute__((ms_abi)) WINAPI *WIN_pa_context_notify_cb_t)(pa_context *c, void *userdata);
typedef void(__attribute__((ms_abi)) WINAPI *WIN_pa_context_success_cb_t)(pa_context *c, int success, void *userdata);
typedef void(__attribute__((ms_abi)) WINAPI *WIN_pa_context_event_cb_t)(pa_context *c, const char *name, pa_proplist *p, void *userdata);
typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_context_subscribe_cb_t)(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata);
typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_sink_input_info_cb_t)(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_stream_success_cb_t)(pa_stream *s, int success, void *userdata);
typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_stream_request_cb_t)(pa_stream *p, size_t nbytes, void *userdata);
typedef void (__attribute__((ms_abi)) WINAPI *WIN_pa_stream_notify_cb_t)(pa_stream *p, void *userdata);

DECL_FUNC(const char *, pa_strerror, (int err), (err))
DECL_FUNC(pa_threaded_mainloop *, pa_threaded_mainloop_new, (void), ())
DECL_FUNC_VOID(pa_threaded_mainloop_free, (pa_threaded_mainloop *m), (m))
DECL_FUNC(int, pa_threaded_mainloop_start, (pa_threaded_mainloop *m), (m))
DECL_FUNC_VOID(pa_threaded_mainloop_stop, (pa_threaded_mainloop *m), (m))
DECL_FUNC_VOID(pa_threaded_mainloop_lock, (pa_threaded_mainloop *m), (m))
DECL_FUNC_VOID(pa_threaded_mainloop_unlock, (pa_threaded_mainloop *m), (m))
DECL_FUNC_VOID(pa_threaded_mainloop_wait, (pa_threaded_mainloop *m), (m))
DECL_FUNC_VOID(pa_threaded_mainloop_signal, (pa_threaded_mainloop *m, int wait_for_accept), (m, wait_for_accept))
DECL_FUNC_VOID(pa_threaded_mainloop_accept, (pa_threaded_mainloop *m), (m))
DECL_FUNC(int, pa_threaded_mainloop_get_retval, (const pa_threaded_mainloop *m), (m))
DECL_FUNC(pa_mainloop_api *, pa_threaded_mainloop_get_api, (pa_threaded_mainloop *m), (m))

DECL_FUNC(pa_stream *, pa_stream_new, (pa_context * c, const char * name, const pa_sample_spec * ss, const pa_channel_map * map), (c, name, ss, map))
DECL_FUNC(int, pa_stream_connect_playback, (pa_stream * s, const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags, const pa_cvolume * volume, pa_stream * sync_stream), (s, dev, attr, flags, volume, sync_stream))
DECL_FUNC(int, pa_stream_disconnect, (pa_stream * s), (s))
DECL_FUNC_VOID(pa_stream_unref, (pa_stream * s), (s))
DECL_FUNC_CB(int, pa_stream_write, (pa_stream * p, const void * userdata, size_t nbytes, WIN_pa_free_cb_t cb, int64_t offset, pa_seek_mode_t seek), (p, cb_data, nbytes, pa_stream_write_cb, offset, seek), void, (void *userdata), (cb_data->userdata))
DECL_FUNC(int, pa_stream_cancel_write, (pa_stream * p), (p))
DECL_FUNC(int, pa_stream_drop, (pa_stream * p), (p))
DECL_FUNC(size_t, pa_stream_writable_size, (const pa_stream * p), (p))
DECL_FUNC_CB(pa_operation *, pa_stream_drain, (pa_stream * s, WIN_pa_stream_success_cb_t cb, void * userdata), (s, pa_stream_drain_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_stream_set_write_callback, (pa_stream * p, WIN_pa_stream_request_cb_t cb, void * userdata), (p, pa_stream_set_write_callback_cb, cb_data), void, (pa_stream *p, size_t nbytes, void *userdata), (p, nbytes, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_stream_set_state_callback, (pa_stream * s, WIN_pa_stream_notify_cb_t cb, void * userdata), (s, pa_stream_set_state_callback_cb, cb_data), void, (pa_stream *p, void *userdata), (p, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_stream_set_started_callback, (pa_stream * p, WIN_pa_stream_notify_cb_t cb, void * userdata), (p, pa_stream_set_started_callback_cb, cb_data), void, (pa_stream *p, void *userdata), (p, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_stream_set_underflow_callback, (pa_stream * p, WIN_pa_stream_notify_cb_t cb, void * userdata), (p, pa_stream_set_underflow_callback_cb, cb_data), void, (pa_stream *p, void *userdata), (p, cb_data->userdata))
DECL_FUNC_CB(pa_operation *, pa_stream_cork, (pa_stream * s, int b, WIN_pa_stream_success_cb_t cb, void * userdata), (s, b, pa_stream_cork_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC(int, pa_stream_is_corked, (const pa_stream * s), (s))
DECL_FUNC_CB(pa_operation *, pa_stream_flush, (pa_stream * s, WIN_pa_stream_success_cb_t cb, void * userdata), (s, pa_stream_flush_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC_CB(pa_operation *, pa_stream_update_sample_rate, (pa_stream * s, uint32_t rate, WIN_pa_stream_success_cb_t cb, void * userdata), (s, rate, pa_stream_update_sample_rate_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC(pa_stream_state_t, pa_stream_get_state, (const pa_stream * p), (p))
DECL_FUNC(const pa_sample_spec *, pa_stream_get_sample_spec, (pa_stream * s), (s))
DECL_FUNC(int, pa_stream_get_latency, (pa_stream * s, pa_usec_t * r_usec, int * negative), (s, r_usec, negative))
DECL_FUNC(const pa_timing_info *, pa_stream_get_timing_info, (pa_stream * s), (s))
DECL_FUNC_CB(pa_operation *, pa_stream_trigger, (pa_stream * s, WIN_pa_stream_success_cb_t cb, void * userdata), (s, pa_stream_trigger_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC_CB(pa_operation *, pa_stream_update_timing_info, (pa_stream * p, WIN_pa_stream_success_cb_t cb, void * userdata), (p, pa_stream_update_timing_info_cb, cb_data), void, (pa_stream *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC(const pa_buffer_attr *, pa_stream_get_buffer_attr, (pa_stream * s), (s))
DECL_FUNC(uint32_t, pa_stream_get_index, (const pa_stream * s), (s))

/* PROPLIST */
DECL_FUNC(pa_proplist *, pa_proplist_new, (void), ())
DECL_FUNC_VOID(pa_proplist_free, (pa_proplist * p), (p))
DECL_FUNC(int, pa_proplist_sets, (pa_proplist * p, const char * key, const char * value), (p, key, value))
DECL_FUNC(int, pa_proplist_set, (pa_proplist * p, const char * key, const void * data, size_t nbytes), (p, key, data, nbytes))
DECL_FUNC(int, pa_proplist_setp, (pa_proplist * p, const char * pair), (p, pair))

/* CONTEXT */
DECL_FUNC(pa_context *, pa_context_new_with_proplist, (pa_mainloop_api * mainloop, const char * name, const pa_proplist * proplist), (mainloop, name, proplist))
DECL_FUNC_VOID(pa_context_unref, (pa_context * c), (c))
DECL_FUNC(int, pa_context_errno, (const pa_context * c), (c))
DECL_FUNC(int, pa_context_connect, (pa_context * c, const char * server, pa_context_flags_t flags, const pa_spawn_api * api), (c, server, flags, api))
DECL_FUNC_VOID(pa_context_disconnect, (pa_context * c), (c))
DECL_FUNC(pa_context_state_t, pa_context_get_state, (const pa_context * c), (c))
DECL_FUNC_CB_VOID(pa_context_set_state_callback, (pa_context * c, WIN_pa_context_notify_cb_t cb, void * userdata), (c, pa_context_set_state_callback_cb, cb_data), void, (pa_context *c, void *userdata), (c, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_context_set_event_callback, (pa_context * p, WIN_pa_context_event_cb_t cb, void * userdata), (p, pa_context_set_event_callback_cb, cb_data), void, (pa_context *c, const char *name, pa_proplist *p, void *userdata), (c, name, p, cb_data->userdata))
DECL_FUNC_CB(pa_operation *, pa_context_get_sink_input_info, (pa_context * c, uint32_t idx, WIN_pa_sink_input_info_cb_t cb, void * userdata), (c, idx, pa_context_get_sink_input_info_cb, cb_data), void, (pa_context *c, const pa_sink_input_info *i, int eol, void *userdata), (c, i, eol, cb_data->userdata))
DECL_FUNC_CB(pa_operation *, pa_context_subscribe, (pa_context * c, pa_subscription_mask_t m, WIN_pa_context_success_cb_t cb, void * userdata), (c, m, pa_context_subscribe_cb, cb_data), void, (pa_context *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC_CB_VOID(pa_context_set_subscribe_callback, (pa_context * c, WIN_pa_context_subscribe_cb_t cb, void * userdata), (c, pa_context_set_subscribe_callback_cb, cb_data), void, (pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata), (c, t, idx, cb_data->userdata))

/* CHANNEL */
DECL_FUNC(pa_channel_map *, pa_channel_map_init_auto, (pa_channel_map * m, unsigned channels, pa_channel_map_def_t def), (m, channels, def))

/* OPERATION */
DECL_FUNC_VOID(pa_operation_unref, (pa_operation * o), (o))
DECL_FUNC(pa_operation_state_t, pa_operation_get_state, (const pa_operation * o), (o))

/* SAMPLE */
DECL_FUNC(pa_usec_t, pa_bytes_to_usec, (uint64_t length, const pa_sample_spec * spec), (length, spec))
DECL_FUNC(size_t, pa_usec_to_bytes, (pa_usec_t t, const pa_sample_spec * spec), (t, spec))

/* VOLUME */
DECL_FUNC(pa_volume_t, pa_sw_volume_from_dB, (double f), (f))
DECL_FUNC(double, pa_sw_volume_to_dB, (pa_volume_t v), (v))
DECL_FUNC_CB(pa_operation *, pa_context_set_sink_input_volume, (pa_context * c, uint32_t idx, const pa_cvolume * volume, WIN_pa_context_success_cb_t cb, void * userdata), (c, idx, volume, pa_context_set_sink_input_volume_cb, cb_data), void, (pa_context *c, int success, void *userdata), (c, success, cb_data->userdata))
DECL_FUNC(pa_cvolume *, pa_cvolume_init, (pa_cvolume * a), (a))
DECL_FUNC(pa_cvolume *, pa_cvolume_set, (pa_cvolume * a, unsigned channels, pa_volume_t v), (a, channels, v))
DECL_FUNC(int, pa_cvolume_valid, (const pa_cvolume * v), (v))
DECL_FUNC(int, pa_cvolume_equal, (const pa_cvolume * a, const pa_cvolume * b), (a, b))

#undef DECL_FUNC
#undef DECL_FUNC_VOID
#undef DECL_FUNC_CB
#undef DECL_FUNC_CB_VOID
