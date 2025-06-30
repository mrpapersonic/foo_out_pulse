# (c) paper 2025
# under GNU GPLv3

# NOTE: parameters of size_t have been replaced with ptr.
# I'm not entirely sure whether this is correct or not.

# strerror
@ stdcall WIN_pa_strerror (long)

# threaded_mainloop
@ stdcall WIN_pa_threaded_mainloop_new ()
@ stdcall WIN_pa_threaded_mainloop_free (ptr)
@ stdcall WIN_pa_threaded_mainloop_start (ptr)
@ stdcall WIN_pa_threaded_mainloop_stop (ptr)
@ stdcall WIN_pa_threaded_mainloop_lock (ptr)
@ stdcall WIN_pa_threaded_mainloop_unlock (ptr)
@ stdcall WIN_pa_threaded_mainloop_wait (ptr)
@ stdcall WIN_pa_threaded_mainloop_signal (ptr long)
@ stdcall WIN_pa_threaded_mainloop_accept (ptr)
@ stdcall WIN_pa_threaded_mainloop_get_retval (ptr)
@ stdcall WIN_pa_threaded_mainloop_get_api (ptr)
@ stdcall WIN_pa_threaded_mainloop_set_name (ptr str)

# stream
@ stdcall WIN_pa_stream_new (ptr str ptr ptr)
@ stdcall WIN_pa_stream_connect_playback (ptr str ptr long ptr ptr)
@ stdcall WIN_pa_stream_disconnect (ptr)
@ stdcall WIN_pa_stream_unref (ptr)
# 3rd param is size_t
@ stdcall WIN_pa_stream_write (ptr ptr ptr ptr int64 long)
@ stdcall WIN_pa_stream_cancel_write (ptr)
@ stdcall WIN_pa_stream_drop (ptr)
@ stdcall WIN_pa_stream_writable_size (ptr)
@ stdcall WIN_pa_stream_drain (ptr ptr ptr)
@ stdcall WIN_pa_stream_set_write_callback (ptr ptr ptr)
@ stdcall WIN_pa_stream_set_state_callback (ptr ptr ptr)
@ stdcall WIN_pa_stream_set_started_callback (ptr ptr ptr)
@ stdcall WIN_pa_stream_set_underflow_callback (ptr ptr ptr)
@ stdcall WIN_pa_stream_cork (ptr long ptr ptr)
@ stdcall WIN_pa_stream_is_corked (ptr)
@ stdcall WIN_pa_stream_flush (ptr ptr ptr)
@ stdcall WIN_pa_stream_update_sample_rate (ptr long ptr ptr)
@ stdcall WIN_pa_stream_get_state (ptr)
@ stdcall WIN_pa_stream_get_sample_spec (ptr)
@ stdcall WIN_pa_stream_get_latency (ptr ptr ptr)
@ stdcall WIN_pa_stream_get_timing_info (ptr)
@ stdcall WIN_pa_stream_trigger (ptr ptr ptr)
@ stdcall WIN_pa_stream_update_timing_info (ptr ptr ptr)
@ stdcall WIN_pa_stream_get_buffer_attr (ptr)
@ stdcall WIN_pa_stream_get_index (ptr)

# PROPLIST
@ stdcall WIN_pa_proplist_new ()
@ stdcall WIN_pa_proplist_free (ptr)
@ stdcall WIN_pa_proplist_sets (ptr str str)
# 4th param is size_t
@ stdcall WIN_pa_proplist_set (ptr str str ptr)
@ stdcall WIN_pa_proplist_setp (ptr str)

# CONTEXT
@ stdcall WIN_pa_context_new_with_proplist (ptr str ptr)
@ stdcall WIN_pa_context_unref (ptr)
@ stdcall WIN_pa_context_errno (ptr)
@ stdcall WIN_pa_context_connect (ptr str long ptr)
@ stdcall WIN_pa_context_disconnect (ptr)
@ stdcall WIN_pa_context_get_state (ptr)
@ stdcall WIN_pa_context_set_state_callback (ptr ptr ptr)
@ stdcall WIN_pa_context_set_event_callback (ptr ptr ptr)
@ stdcall WIN_pa_context_get_sink_input_info (ptr long ptr ptr)
# 2nd param is pa_subscription_mask_t (enum)
@ stdcall WIN_pa_context_subscribe (ptr long ptr ptr)
@ stdcall WIN_pa_context_set_subscribe_callback (ptr ptr ptr)

# CHANNEL
# 3rd param is enum (pa_channel_map_def_t)
@ stdcall WIN_pa_channel_map_init_auto (ptr long long)

# OPERATION
@ stdcall WIN_pa_operation_unref (ptr)
@ stdcall WIN_pa_operation_get_state (ptr)

# SAMPLE
@ stdcall WIN_pa_bytes_to_usec (int64 ptr)
@ stdcall WIN_pa_usec_to_bytes (int64 ptr)

# VOLUME
@ stdcall WIN_pa_sw_volume_from_dB (double)
# param is pa_volume_t
@ stdcall WIN_pa_sw_volume_to_dB (long)
@ stdcall WIN_pa_context_set_sink_input_volume (ptr long ptr ptr ptr)
@ stdcall WIN_pa_cvolume_init (ptr)
@ stdcall WIN_pa_cvolume_set (ptr long long)
@ stdcall WIN_pa_cvolume_valid (ptr)
@ stdcall WIN_pa_cvolume_equal (ptr ptr)

# dummy func, to tell foo_out_pulse about us
@ stdcall foo_out_pulse_winelib_dll ()

# End of file
