#include "stdafx.h"
// PCH ^

#include <pathcch.h>
#include <windows.h>

#include <mutex>
#include <sstream>

#include "foobar2000-sdk/foobar2000/SDK/core_api.h"
#include "foobar2000-sdk/foobar2000/SDK/output.h"
#include "pulse.h"

/* cargopasta'd from foobar2000 SDK, was removed after 2.0  --paper */
struct t_samplespec {
    t_uint32 m_sample_rate;
    t_uint32 m_channels, m_channel_config;

    t_size time_to_samples(double p_time) const { PFC_ASSERT(is_valid()); return (t_size)audio_math::time_to_samples(p_time, m_sample_rate); }
    double samples_to_time(t_size p_samples) const { PFC_ASSERT(is_valid()); return audio_math::samples_to_time(p_samples, m_sample_rate); }

    inline t_samplespec() { reset(); }
    inline t_samplespec(audio_chunk const& in) { fromchunk(in); }

    inline void reset() { m_sample_rate = 0; m_channels = 0; m_channel_config = 0; }

    inline bool operator==(const t_samplespec& p_spec2) const {
        return m_sample_rate == p_spec2.m_sample_rate && m_channels == p_spec2.m_channels && m_channel_config == p_spec2.m_channel_config;
    }

    inline bool operator!=(const t_samplespec& p_spec2) const {
        return !(*this == p_spec2);
    }

    inline bool is_valid() const {
        return m_sample_rate > 0 && m_channels > 0 && audio_chunk::g_count_channels(m_channel_config) == m_channels;
    }

    static t_samplespec g_fromchunk(const audio_chunk& p_chunk) {
        t_samplespec temp; temp.fromchunk(p_chunk); return temp;
    }

    void fromchunk(const audio_chunk& p_chunk) {
        m_sample_rate = p_chunk.get_sample_rate();
        m_channels = p_chunk.get_channels();
        m_channel_config = p_chunk.get_channel_config();
    }
};

namespace {

static pa_strerror g_pa_strerror;
static pa_threaded_mainloop_new g_pa_threaded_mainloop_new;
static pa_threaded_mainloop_free g_pa_threaded_mainloop_free;
static pa_threaded_mainloop_start g_pa_threaded_mainloop_start;
static pa_threaded_mainloop_stop g_pa_threaded_mainloop_stop;
static pa_threaded_mainloop_lock g_pa_threaded_mainloop_lock;
static pa_threaded_mainloop_unlock g_pa_threaded_mainloop_unlock;
static pa_threaded_mainloop_wait g_pa_threaded_mainloop_wait;
static pa_threaded_mainloop_signal g_pa_threaded_mainloop_signal;
static pa_threaded_mainloop_accept g_pa_threaded_mainloop_accept;
static pa_threaded_mainloop_get_retval g_pa_threaded_mainloop_get_retval;
static pa_threaded_mainloop_get_api g_pa_threaded_mainloop_get_api;
static pa_stream_new g_pa_stream_new;
static pa_stream_connect_playback g_pa_stream_connect_playback;
static pa_stream_disconnect g_pa_stream_disconnect;
static pa_stream_unref g_pa_stream_unref;
static pa_stream_write g_pa_stream_write;
static pa_stream_cancel_write g_pa_stream_cancel_write;
static pa_stream_drop g_pa_stream_drop;
static pa_stream_writable_size g_pa_stream_writable_size;
static pa_stream_drain g_pa_stream_drain;
static pa_stream_set_write_callback g_pa_stream_set_write_callback;
static pa_stream_set_state_callback g_pa_stream_set_state_callback;
static pa_stream_set_started_callback g_pa_stream_set_started_callback;
static pa_stream_set_underflow_callback g_pa_stream_set_underflow_callback;
static pa_stream_cork g_pa_stream_cork;
static pa_stream_is_corked g_pa_stream_is_corked;
static pa_stream_flush g_pa_stream_flush;
static pa_stream_update_sample_rate g_pa_stream_update_sample_rate;
static pa_stream_get_state g_pa_stream_get_state;
static pa_stream_get_sample_spec g_pa_stream_get_sample_spec;
static pa_stream_get_latency g_pa_stream_get_latency;
static pa_stream_get_timing_info g_pa_stream_get_timing_info;
static pa_stream_trigger g_pa_stream_trigger;
static pa_stream_update_timing_info g_pa_stream_update_timing_info;
static pa_stream_get_buffer_attr g_pa_stream_get_buffer_attr;
static pa_proplist_new g_pa_proplist_new;
static pa_proplist_free g_pa_proplist_free;
static pa_proplist_set g_pa_proplist_set;
static pa_proplist_sets g_pa_proplist_sets;
static pa_proplist_setp g_pa_proplist_setp;
static pa_context_new_with_proplist g_pa_context_new_with_proplist;
static pa_context_unref g_pa_context_unref;
static pa_context_errno g_pa_context_errno;
static pa_context_connect g_pa_context_connect;
static pa_context_disconnect g_pa_context_disconnect;
static pa_context_get_state g_pa_context_get_state;
static pa_context_set_state_callback g_pa_context_set_state_callback;
static pa_context_set_event_callback g_pa_context_set_event_callback;
static pa_operation_unref g_pa_operation_unref;
static pa_operation_get_state g_pa_operation_get_state;
static pa_bytes_to_usec g_pa_bytes_to_usec;
static pa_usec_to_bytes g_pa_usec_to_bytes;
static pa_channel_map_init_auto g_pa_channel_map_init_auto;
static pa_stream_get_index g_pa_stream_get_index;
static pa_sw_volume_from_dB g_pa_sw_volume_from_dB;
static pa_sw_volume_to_dB g_pa_sw_volume_to_dB;
static pa_cvolume_valid g_pa_cvolume_valid;
static pa_cvolume_equal g_pa_cvolume_equal;
static pa_context_set_sink_input_volume g_pa_context_set_sink_input_volume;
static pa_cvolume_init g_pa_cvolume_init;
static pa_cvolume_set g_pa_cvolume_set;
static pa_context_get_sink_input_info g_pa_context_get_sink_input_info;
static pa_context_subscribe g_pa_context_subscribe;
static pa_context_set_subscribe_callback g_pa_context_set_subscribe_callback;
static bool g_pa_is_loaded = false;
static bool is_using_winelib = false;

static const GUID guid_cfg_pulseaudio_branch = {
    0x61979096,
    0x1158,
    0x4860,
    {0xb0, 0xcc, 0x6f, 0x53, 0xf, 0x35, 0xaf, 0x26}};
static const GUID guid_cfg_pulseaudio_fade_out_seek = {
    0x319d2507,
    0xe2aa,
    0x40e2,
    {0xa1, 0xec, 0x4e, 0x94, 0xf1, 0xdd, 0x12, 0x8a}};
static const GUID guid_cfg_pulseaudio_fade_in_seek = {
    0x90ae1a07,
    0xcd2b,
    0x481c,
    {0xb2, 0x6a, 0xf7, 0x36, 0x83, 0xec, 0xf6, 0x40}};
static const GUID guid_cfg_pulseaudio_fade_out_track = {
    0xe136e959,
    0x929b,
    0x4005,
    {0xaa, 0x9e, 0x8e, 0x8b, 0x91, 0x5b, 0x5d, 0x2}};
static const GUID guid_cfg_pulseaudio_fade_in_track = {
    0x6fb3670, 0x4e7d, 0x4601, {0x83, 0xa6, 0xed, 0x44, 0x3e, 0xb1, 0xe1, 0x7}};
static const GUID guid_cfg_pulseaudio_minreq_workaround = {
    0xe176bd02, 0xcbc, 0x4fbd, {0x8f, 0x1a, 0xf2, 0x3a, 0x2a, 0xb7, 0x8, 0x86}};
static const GUID guid_cfg_pulseaudio_fade_out_stop = {
    0xbf045192,
    0xde9b,
    0x432d,
    {0xa5, 0xd9, 0x36, 0xb1, 0x19, 0x57, 0x6a, 0x61}};
static const GUID guid_cfg_pulseaudio_prebuffer = {
    0x64cd1e28,
    0x87ea,
    0x41e5,
    {0xaf, 0x3d, 0xc6, 0xcd, 0x2f, 0x52, 0xac, 0xee}};

static advconfig_branch_factory g_pulseaudio_output_branch(
    "PulseAudio output", guid_cfg_pulseaudio_branch,
    advconfig_branch::guid_branch_playback, 0);
static advconfig_integer_factory cfg_pulseaudio_seek_fade_out(
    "Fade out on seek (milliseconds)", guid_cfg_pulseaudio_fade_out_seek,
    guid_cfg_pulseaudio_branch, 0, 15, 0, 1000, 0);
static advconfig_integer_factory cfg_pulseaudio_seek_fade_in(
    "Fade in on seek (milliseconds)", guid_cfg_pulseaudio_fade_in_seek,
    guid_cfg_pulseaudio_branch, 0, 15, 0, 1000, 0);
static advconfig_integer_factory cfg_pulseaudio_track_fade_out(
    "Fade out on manual track change (milliseconds)",
    guid_cfg_pulseaudio_fade_out_track, guid_cfg_pulseaudio_branch, 0, 0, 0,
    1000, 0);
static advconfig_integer_factory cfg_pulseaudio_track_fade_in(
    "Fade in on manual track change (milliseconds)",
    guid_cfg_pulseaudio_fade_in_track, guid_cfg_pulseaudio_branch, 0, 0, 0,
    1000, 0);
static advconfig_checkbox_factory cfg_pulseaudio_minreq_workaround(
    "Enable workaround for driver issue", guid_cfg_pulseaudio_minreq_workaround,
    guid_cfg_pulseaudio_branch, 0, false);
static advconfig_integer_factory cfg_pulseaudio_prebuf(
    "Request prebuffer (milliseconds)", guid_cfg_pulseaudio_prebuffer,
    guid_cfg_pulseaudio_branch, 0, 200, 0, 100000, 0);

/* LOL WOW C++ IS UGLY */
class lookback_buffer {
 public:
  lookback_buffer() : max_size_(0), buf_(), out_buf_(), lookback_(0){};

  void queue(void* in, size_t nbytes) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    pfc::dynamic_assert(nbytes <= max_size_);
    size_t endspace = max_size_ - head_;
    memcpy(buf_.get() + head_, in, pfc::min_t(nbytes, endspace));
    if (endspace < nbytes) {
      memcpy(buf_.get(), (BYTE*)in + endspace, nbytes - endspace);
    }
    head_ = (head_ + nbytes) % max_size_;
    lookback_ = pfc::min_t(max_size_, lookback_ + nbytes);
  }

  size_t read_back(size_t distance) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    size_t to_read = pfc::min_t(distance, lookback_);
    if (head_ < to_read) {
      memcpy(out_buf_.get(), buf_.get() + max_size_ - (to_read - head_),
             to_read - head_);
      memcpy(out_buf_.get() + to_read - head_, buf_.get(), head_);
    } else {
      memcpy(out_buf_.get(), buf_.get() + head_ - to_read, to_read);
    }
    head_ = 0;
    lookback_ = 0;

    return to_read;
  }

  void reset(size_t size) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    head_ = 0;
    lookback_ = 0;
    buf_ = std::unique_ptr<BYTE>(new BYTE[size]);
    out_buf_ = std::shared_ptr<BYTE>(new BYTE[size]);
    max_size_ = size;
  }

  void reset() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    head_ = 0;
    lookback_ = 0;
  }

  std::shared_ptr<BYTE> out_buf_;

 private:
  std::mutex buffer_mutex_;
  std::unique_ptr<BYTE> buf_;
  size_t head_ = 0;
  size_t lookback_ = 0;
  size_t max_size_;
};

class output_pulse : public output_v4 {
 public:
  typedef struct fade_in {
    bool active = false;
    size_t total_samples = 0;
    size_t progress = 0;
  } fade;

  output_pulse(const GUID& p_device, double p_buffer_length, bool p_dither,
               t_uint32 p_bitdepth)
      : buffer_length(p_buffer_length),
        m_incoming_ptr(0),
        progressing(false),
        draining(false),
        drained(false),
        cfg_seek_fade_out(pfc::min_t((size_t)cfg_pulseaudio_seek_fade_out,
                                     (size_t)(1000 * p_buffer_length))),
        cfg_seek_fade_in((size_t)cfg_pulseaudio_seek_fade_in),
        cfg_track_fade_out(pfc::min_t((size_t)cfg_pulseaudio_track_fade_out,
                                      (size_t)(1000 * p_buffer_length))),
        cfg_track_fade_in((size_t)cfg_pulseaudio_track_fade_in),
        fade_in_next_ms(0),
        active_fade_in(),
        rewind_buffer(),
        rewind_active(cfg_pulseaudio_seek_fade_out > 0 ||
                      cfg_pulseaudio_track_fade_out > 0),
        next_write_relative(false),
        volume(0) {
    if (!load_pulse_dll()) {
      stop();
      return;
    }

    mainloop = g_pa_threaded_mainloop_new();
    if (g_pa_threaded_mainloop_start(mainloop) < 0) {
      g_pa_threaded_mainloop_free(mainloop);
      mainloop = NULL;
      console_error("failed to start playback thread", 0);
      stop();
      return;
    }


    g_pa_threaded_mainloop_lock(mainloop);
    pa_mainloop_api *api = g_pa_threaded_mainloop_get_api(mainloop);
    {
        pa_proplist* proplist = g_pa_proplist_new();
        g_pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "foobar2000");
        g_pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "foobar2000");
        g_pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "foobar2000");
        // FIXME how do we do this
        // g_pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "");
        context = g_pa_context_new_with_proplist(api, "foobar2000", proplist);
        //if (proplist != NULL) g_pa_proplist_free(proplist);
    }

    g_pa_context_set_state_callback(context, context_state_cb, this);
    const char* server = /*is_using_winelib ? NULL : */"127.0.0.1";
    if (g_pa_context_connect(context, server, (pa_context_flags_t)0, NULL) < 0 || context_wait(context, mainloop)) {
      g_pa_context_unref(context);
      context = NULL;
      g_pa_threaded_mainloop_unlock(mainloop);
      g_pa_threaded_mainloop_stop(mainloop);
      g_pa_threaded_mainloop_free(mainloop);
      mainloop = NULL;

      stop();
      return;
    }

    pa_operation* op = g_pa_context_subscribe(
        context, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
    if (op != NULL) {
      g_pa_operation_unref(op);
    }
    g_pa_context_set_subscribe_callback(context, context_subscribe_cb, this);

    g_pa_threaded_mainloop_unlock(mainloop);

    trigger_update.create(true, true);
  }
  ~output_pulse() {
    if (mainloop != NULL) {
      g_pa_threaded_mainloop_lock(mainloop);
      if (context != NULL) {
        g_pa_context_disconnect(context);
        g_pa_context_set_event_callback(context, NULL, NULL);
        g_pa_context_set_state_callback(context, NULL, NULL);
        g_pa_context_unref(context);
      }
      g_pa_threaded_mainloop_unlock(mainloop);

      g_pa_threaded_mainloop_stop(mainloop);
      Sleep(100);  // _stop() doesn't seem to block until it's actually safe to
                   // free the mainloop?
      g_pa_threaded_mainloop_free(mainloop);
    }
  }

  void pause(bool p_state) {
    if (stream == NULL) return;

    g_pa_threaded_mainloop_lock(mainloop);
    pa_operation* op = g_pa_stream_cork(stream, p_state ? 1 : 0, NULL, NULL);
    if (op != NULL) {
      g_pa_operation_unref(op);
    }
    g_pa_threaded_mainloop_unlock(mainloop);
  }

  void volume_set(double p_val) {
    if (stream == NULL) return;

    pa_volume_t new_volume = g_pa_sw_volume_from_dB(p_val);
    if (new_volume != volume) {
      volume = new_volume;
      uint32_t index = g_pa_stream_get_index(stream);
      pa_cvolume cvolume;
      g_pa_cvolume_init(&cvolume);
      cvolume.channels = m_active_spec.m_channels;
      g_pa_cvolume_set(&cvolume, m_active_spec.m_channels, volume);

      g_pa_threaded_mainloop_lock(mainloop);
      pa_operation* op = g_pa_context_set_sink_input_volume(
          context, index, &cvolume, NULL, NULL);
      if (op != NULL) {
        g_pa_operation_unref(op);
      }
      g_pa_threaded_mainloop_unlock(mainloop);
    }
  }

  void flush() {
    m_incoming_ptr = 0;
    m_incoming.set_size(0);

    write_fade_out(cfg_seek_fade_out);
    active_fade_in = fade();
    fade_in_next_ms = cfg_seek_fade_in;
  }

  void flush_changing_track() {
    m_incoming_ptr = 0;
    m_incoming.set_size(0);

    write_fade_out(cfg_track_fade_out);
    active_fade_in = fade();
    fade_in_next_ms = cfg_track_fade_in;
  }

  void update(bool& p_ready) { p_ready = update_v2() > 0; }

  size_t update_v2() {
    trigger_update.set_state(false);

    if (m_incoming_spec != m_active_spec) {
      if (drained || next_write_relative) {
        next_write_relative = false;
        drained = false;
        open_incoming_spec();
      } else {
        force_play();
      }
    }

    size_t retCanWriteSamples = 0;
    if (m_incoming_spec == m_active_spec &&
        m_incoming_ptr < m_incoming.get_size()) {
      retCanWriteSamples = write();
    } else if (m_incoming_ptr == m_incoming.get_size()) {
      retCanWriteSamples = SIZE_MAX;
    }
    return retCanWriteSamples;
  }

  void force_play() {
    if (draining) return;

    if (stream != NULL) {
      g_pa_threaded_mainloop_lock(mainloop);
      draining = true;
      drained = false;
      pa_operation* op = g_pa_stream_drain(stream, stream_drained_cb, this);
      if (op != NULL) {
        g_pa_operation_unref(op);
      } else {
        // nothing to drain
        draining = false;
        drained = true;
      }
      op = g_pa_stream_trigger(stream, NULL, NULL);
      if (op != NULL) {
        g_pa_operation_unref(op);
      }
      g_pa_threaded_mainloop_unlock(mainloop);
    } else {
      draining = false;
      drained = true;
    }
  }

  double get_latency() {
    double ret = 0;
    if (m_incoming_spec.is_valid()) {
      ret += audio_math::samples_to_time(
          (m_incoming.get_size() - m_incoming_ptr) / m_incoming_spec.m_channels,
          m_incoming_spec.m_sample_rate);
    }
    if (m_active_spec.is_valid() && !drained) {
      if (stream != NULL) {
        pa_usec_t latency;
        const pa_timing_info* timing_info = g_pa_stream_get_timing_info(stream);
        if (g_pa_stream_get_latency(stream, &latency, NULL) > -1) {
          ret += (latency * 0.000001);
        } else {
          g_pa_threaded_mainloop_lock(mainloop);
          pa_operation* op = g_pa_stream_update_timing_info(
              stream, stream_success_cb, mainloop);
          wait_for_op(op);
          g_pa_threaded_mainloop_unlock(mainloop);
          if (g_pa_stream_get_latency(stream, &latency, NULL) > -1) {
            ret += (latency * 0.000001);
          }
        }
      }
    }
    return ret;
  }

  void process_samples(const audio_chunk& p_chunk) {
    pfc::dynamic_assert(m_incoming_ptr == m_incoming.get_size());
    t_samplespec spec;
    spec.fromchunk(p_chunk);
    if (!spec.is_valid())
      pfc::throw_exception_with_message<exception_io_data>(
          "Invalid audio stream specifications");
    m_incoming_spec = spec;
    size_t length = p_chunk.get_used_size();
    m_incoming.set_data_fromptr(p_chunk.get_data(), length);
    m_incoming_ptr = 0;

    if (fade_in_next_ms > 0) {
      active_fade_in = fade();
      active_fade_in.active = true;
      active_fade_in.total_samples = m_incoming_spec.time_to_samples(0.001 * fade_in_next_ms);
      active_fade_in.progress = 0;
      fade_in_next_ms = 0;
    }

    if (active_fade_in.active) {
      size_t fade_samples =
          pfc::min_t(m_incoming.get_size() / m_incoming_spec.m_channels,
                     (active_fade_in.total_samples - active_fade_in.progress));
      fade_section(m_incoming.get_ptr(), fade_samples,
                   active_fade_in.total_samples, active_fade_in.progress,
                   m_incoming_spec.m_channels, true);
      active_fade_in.progress += fade_samples;
      if (active_fade_in.progress == active_fade_in.total_samples)
        active_fade_in.active = false;
    }
  }

  bool is_progressing() { return progressing; }

  pfc::eventHandle_t get_trigger_event() { return trigger_update.get_handle(); }

  static void g_enum_devices(output_device_enum_callback& p_callback) {
    const GUID device = {0x8bf1c19,
                         0x5b9d,
                         0x4992,
                         {0x76, 0x18, 0x13, 0x8b, 0xa2, 0x1, 0xd7, 0xa6}};
    if (load_pulse_dll()) {
      if (is_using_winelib) {
        p_callback.on_device(device, "native", 6);
      } else {
        p_callback.on_device(device, "localhost", 9);
      }
    }
  }
  static GUID g_get_guid() {
    static const GUID guid = {0xfe94df9,
                              0xc8e2,
                              0x40a1,
                              {0x40, 0xa1, 0xb1, 0x2a, 0x4a, 0x6c, 0xe4, 0x9e}};
    return guid;
  }
  static bool g_advanced_settings_query() { return false; }
  static bool g_needs_bitdepth_config() { return false; }
  static bool g_needs_dither_config() { return false; }
  static bool g_needs_device_list_prefixes() { return true; }
  static bool g_supports_multiple_streams() { return false; }
  static bool g_is_high_latency() { return true; }
  static uint32_t g_extra_flags() { return 0; }
  static void g_advanced_settings_popup(HWND p_parent, POINT p_menupoint) {}
  static const char* g_get_name() { return "PulseAudio"; }

 private:
  const double offset = 0.05;

  pa_context* context = NULL;
  pa_threaded_mainloop* mainloop = NULL;
  pa_stream* stream = NULL;

  pfc::array_t<audio_sample, pfc::alloc_fast_aggressive> m_incoming;
  size_t m_incoming_ptr;
  t_samplespec m_incoming_spec, m_active_spec;

  double buffer_length;

  pa_volume_t volume;

  bool progressing;
  bool draining;
  bool drained;

  bool rewind_active;
  lookback_buffer rewind_buffer;
  const size_t cfg_seek_fade_in;
  const size_t cfg_seek_fade_out;
  const size_t cfg_track_fade_in;
  const size_t cfg_track_fade_out;
  size_t fade_in_next_ms;
  fade active_fade_in;
  bool next_write_relative;

  pfc::event trigger_update;

  service_ptr_t<playback_control> playback_control;

  static bool context_wait(pa_context* ctx, pa_threaded_mainloop* ml) {
    pa_context_state_t state;
    while ((state = g_pa_context_get_state(ctx)) != PA_CONTEXT_READY) {
      if (state == PA_CONTEXT_FAILED || state == PA_CONTEXT_TERMINATED)
        return false;
      g_pa_threaded_mainloop_wait(ml);
    }
    return 0;
  }

  static void __stdcall context_subscribe_cb(pa_context* c,
                                   pa_subscription_event_type_t t, uint32_t idx,
                                   void* userdata) {
    if ((pa_subscription_event_type)(t & PA_SUBSCRIPTION_EVENT_SINK_INPUT) ==
        PA_SUBSCRIPTION_EVENT_SINK_INPUT) {
      output_pulse* output = (output_pulse*)userdata;
      if (output->stream == NULL) return;

      if (g_pa_stream_get_index(output->stream) == idx) {
        g_pa_context_get_sink_input_info(output->context, idx,
                                         sink_input_info_cb, output);
      }
    }
  }

  static void __stdcall sink_input_info_cb(pa_context* c, const pa_sink_input_info* i,
                                 int eol, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    if (i == NULL || output == NULL) return;

    if (g_pa_cvolume_valid(&i->volume) &&
        output->volume != i->volume.values[0]) {
      float volume_db = (float)g_pa_sw_volume_to_dB(i->volume.values[0]);
      fb2k::inMainThread(
          [volume_db]() { playback_control::get()->set_volume(volume_db); });
    }
  }

  static void stop() {
    fb2k::inMainThread([]() { playback_control::get()->stop(); });
  }

  static void __stdcall context_state_cb(pa_context* ctx, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    std::stringstream s;
    switch (g_pa_context_get_state(ctx)) {
      case PA_CONTEXT_FAILED:
        console_error("connection failed", g_pa_context_errno(ctx));
        stop();
      case PA_CONTEXT_READY:
      case PA_CONTEXT_TERMINATED:
        g_pa_threaded_mainloop_signal(output->mainloop, 0);
      default:
        break;
    }
  }

  static int stream_wait(pa_stream* s, pa_threaded_mainloop* ml) {
    pa_stream_state_t state;

    while ((state = g_pa_stream_get_state(s)) != PA_STREAM_READY) {
      if (state == PA_STREAM_FAILED || state == PA_STREAM_TERMINATED) return -1;
      g_pa_threaded_mainloop_wait(ml);
    }
    return 0;
  }

  static void __stdcall stream_state_cb(pa_stream* s, void* userdata) {
    pa_threaded_mainloop* ml = (pa_threaded_mainloop*)userdata;

    switch (g_pa_stream_get_state(s)) {
      case PA_STREAM_READY:
      case PA_STREAM_FAILED:
      case PA_STREAM_TERMINATED:
        g_pa_threaded_mainloop_signal(ml, 0);
      default:
        break;
    }
  }

  static void __stdcall stream_started_cb(pa_stream* s, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    output->progressing = true;
  }

  static void __stdcall stream_underflow_cb(pa_stream* s, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    output->progressing = false;
    output->trigger_update.set_state(true);
  }

  static void __stdcall stream_write_cb(pa_stream* s, size_t nbytes, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    output->trigger_update.set_state(true);
  }

  size_t write() {
    if (stream == NULL || m_incoming_spec != m_active_spec) {
      return 0;
    }

    g_pa_threaded_mainloop_lock(mainloop);

    if (next_write_relative) {
      const pa_timing_info* info = g_pa_stream_get_timing_info(stream);
      if (info == NULL) {
        console_error("error writing to stream - no timing info", 0);
        g_pa_threaded_mainloop_unlock(mainloop);
        return 0;
      }

      int64_t write_index = info->read_index -
                            (info->read_index % (4 * m_active_spec.m_channels));

      const pa_buffer_attr* buffer_attr = g_pa_stream_get_buffer_attr(stream);
      if (buffer_attr == NULL) {
        console_error("error writing to stream - no buffer attributes", 0);
        g_pa_threaded_mainloop_unlock(mainloop);
        return 0;
      }

      size_t cw_samples = buffer_attr->tlength / sizeof(audio_sample);
      size_t delta =
          pfc::min_t(m_incoming.get_size() - m_incoming_ptr, cw_samples);
      if (delta > 0) {
        int error = g_pa_stream_write(
            stream, m_incoming.get_ptr() + m_incoming_ptr,
            delta * sizeof(audio_sample), NULL, write_index, PA_SEEK_ABSOLUTE);
        if (error < 0) {
          console_error("error writing to stream", error);
          g_pa_threaded_mainloop_unlock(mainloop);
          return (cw_samples - delta) / m_incoming_spec.m_channels;
        } else {
          next_write_relative = false;

          if (rewind_active)
            rewind_buffer.queue(m_incoming.get_ptr() + m_incoming_ptr,
                                delta * sizeof(audio_sample));

          m_incoming_ptr += delta;
        }
      }

      g_pa_threaded_mainloop_unlock(mainloop);
      return (cw_samples - delta) / m_incoming_spec.m_channels;
    } else {
      size_t cw_samples =
          g_pa_stream_writable_size(stream) / sizeof(audio_sample);
      if (cw_samples == (size_t)-1) {
        console_error("g_pa_stream_writable_size error",
                      g_pa_context_errno(context));
        return 0;
      }

      size_t delta =
          pfc::min_t(m_incoming.get_size() - m_incoming_ptr, cw_samples);

      if (delta > 0) {
        int error = g_pa_stream_write(
            stream, m_incoming.get_ptr() + m_incoming_ptr,
            delta * sizeof(audio_sample), NULL, 0, PA_SEEK_RELATIVE);
        if (error < 0) {
          console_error("error writing to stream", error);
          g_pa_threaded_mainloop_unlock(mainloop);
          return 0;
        } else {
          if (rewind_active)
            rewind_buffer.queue(m_incoming.get_ptr() + m_incoming_ptr,
                                delta * sizeof(audio_sample));

          m_incoming_ptr += delta;
        }
      }

      g_pa_threaded_mainloop_unlock(mainloop);
      return (cw_samples - delta) / m_incoming_spec.m_channels;
    }
  }

  void fade_section(audio_sample* data, size_t num_samples_to_write,
                    size_t total_fade_samples, size_t start_at_sample,
                    size_t num_channels, bool fade_in) {
    if (fade_in) {
      for (size_t s = 0; s < num_samples_to_write; s++) {
        audio_math::scale(
            data + (s * num_channels), num_channels, data + (s * num_channels),
            (1.0f * (s + start_at_sample)) / (1.0f * total_fade_samples));
      }
    } else {
      for (size_t s = 0; s < num_samples_to_write; s++) {
        audio_math::scale(
            data + (s * num_channels), num_channels, data + (s * num_channels),
            (1.0f * (total_fade_samples - (start_at_sample + s))) /
                (1.0f * total_fade_samples));
      }
    }
  }

  void write_fade_out(size_t fade_ms) {
    if (stream == NULL || fade_ms == 0 || !rewind_active) {
      next_write_relative = true;
      trigger_update.set_state(true);
      return;
    }

    g_pa_threaded_mainloop_lock(mainloop);

    if (g_pa_stream_is_corked(stream)) {
      pa_operation* op = g_pa_stream_flush(stream, NULL, NULL);
      if (op != NULL) {
        g_pa_operation_unref(op);
      }

      g_pa_threaded_mainloop_unlock(mainloop);
      return;
    }

    pa_operation* op =
        g_pa_stream_update_timing_info(stream, stream_success_cb, mainloop);
    wait_for_op(op);

    const pa_timing_info* timing_info = g_pa_stream_get_timing_info(stream);
    if (timing_info == NULL) {
      console_error("error writing to stream - no timing info", 0);
      g_pa_threaded_mainloop_unlock(mainloop);
      return;
    }

    const pa_buffer_attr* buffer_attr = g_pa_stream_get_buffer_attr(stream);
    if (buffer_attr == NULL) {
      console_error("error writing to stream - no buffer attributes", 0);
      g_pa_threaded_mainloop_unlock(mainloop);
      return;
    }

    int64_t read_index = timing_info->read_index;
    int64_t write_index = timing_info->write_index;

    int64_t offset_bytes = (int64_t)m_active_spec.time_to_samples(offset) *
                           m_active_spec.m_channels * 4;
    int64_t buffered_bytes =
        (buffer_attr->maxlength + write_index - read_index) %
        buffer_attr->maxlength;

    int64_t rewind_bytes =
        pfc::max_t(buffered_bytes - offset_bytes, (int64_t)0);
    rewind_bytes = rewind_buffer.read_back((size_t)rewind_bytes);

    if (rewind_bytes > 0) {
      std::shared_ptr<BYTE> rewind_data = rewind_buffer.out_buf_;
      int64_t fade_samples =
          pfc::min_t(rewind_bytes / 4 / m_active_spec.m_channels,
                     (int64_t)m_active_spec.time_to_samples(0.001 * fade_ms) *
                         m_active_spec.m_channels);
      fade_section((audio_sample*)rewind_data.get(), (size_t)fade_samples,
                   (size_t)fade_samples, 0, m_active_spec.m_channels, false);

      int64_t write_bytes =
          fade_samples * m_active_spec.m_channels * sizeof(audio_sample);
      int error = g_pa_stream_write(
          stream, (audio_sample*)rewind_data.get(), (size_t)write_bytes, NULL,
          read_index + offset_bytes, PA_SEEK_ABSOLUTE);
      if (error < 0) {
        console_error("error writing fade to stream", error);
      } else {
        rewind_buffer.queue((audio_sample*)rewind_data.get(),
                            (size_t)write_bytes);
        pa_operation* op =
            g_pa_stream_drain(stream, stream_success_cb, mainloop);
        wait_for_op(op);
      }
    } else {
      next_write_relative = true;
    }

    g_pa_threaded_mainloop_unlock(mainloop);
  }

  static void __stdcall stream_success_cb(pa_stream* s, int success, void* userdata) {
    g_pa_threaded_mainloop_signal((pa_threaded_mainloop*)userdata, 0);
  }

  void wait_for_op(pa_operation* op) {
    if (op != NULL) {
      while (g_pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        g_pa_threaded_mainloop_wait(mainloop);
      g_pa_operation_unref(op);
    }
  }

  void close_stream() {
    if (stream != NULL) {
      g_pa_stream_set_state_callback(stream, NULL, NULL);
      g_pa_stream_set_started_callback(stream, NULL, NULL);
      g_pa_stream_set_underflow_callback(stream, NULL, NULL);
      g_pa_stream_set_write_callback(stream, NULL, NULL);
      g_pa_stream_disconnect(stream);
      g_pa_stream_unref(stream);
      stream = NULL;
      progressing = false;
    }
  }

  void open_incoming_spec() {
    if (!m_incoming_spec.is_valid()) return;

    pa_sample_spec ss;
    ss.channels = m_incoming_spec.m_channels;
    ss.rate = m_incoming_spec.m_sample_rate;
    ss.format = PA_SAMPLE_FLOAT32LE;

    struct pa_channel_map map;
    const pa_channel_map* map_ptr =
        g_pa_channel_map_init_auto(&map, ss.channels, PA_CHANNEL_MAP_WAVEEX);

    pa_stream_flags_t flags = (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING |
                                                  PA_STREAM_AUTO_TIMING_UPDATE);

    struct pa_buffer_attr attr;
    attr.maxlength =
        (uint32_t)ceil(m_incoming_spec.time_to_samples(buffer_length + offset) *
                       m_incoming_spec.m_channels * 4);
    attr.fragsize = 0;
    attr.minreq = cfg_pulseaudio_minreq_workaround.get() ? attr.maxlength / 2
                                                         : (uint32_t)-1;
    attr.tlength = attr.maxlength;
    attr.prebuf = (uint32_t)ceil(
        m_incoming_spec.time_to_samples(0.001 * cfg_pulseaudio_prebuf) *
        m_incoming_spec.m_channels * 4);

    std::stringstream s;
    s << "PulseAudio: requesting buffer attributes: maxlength "
      << attr.maxlength << ", minreq " << attr.minreq << ", tlength "
      << attr.tlength << ", prebuf " << attr.prebuf;
    console::info(s.str().c_str());

    g_pa_threaded_mainloop_lock(mainloop);

    close_stream();

    stream = g_pa_stream_new(context, "Audio", &ss, map_ptr);
    progressing = false;
    if (stream == NULL) {
      g_pa_threaded_mainloop_unlock(mainloop);
      console_error("failed to create stream", 0);
      stop();
      return;
    }

    g_pa_stream_set_state_callback(stream, stream_state_cb, mainloop);
    g_pa_stream_set_started_callback(stream, stream_started_cb, this);
    g_pa_stream_set_underflow_callback(stream, stream_underflow_cb, this);
    g_pa_stream_set_write_callback(stream, stream_write_cb, this);

    int err =
        g_pa_stream_connect_playback(stream, NULL, &attr, flags, NULL, NULL);
    if (err < 0 || stream_wait(stream, mainloop)) {
      g_pa_threaded_mainloop_unlock(mainloop);
      console_error("failed to connect stream", err);
      stop();
      return;
    }

    m_active_spec = m_incoming_spec;

    if (rewind_active) {
      const pa_buffer_attr* received_attr = g_pa_stream_get_buffer_attr(stream);
      if (received_attr == NULL) {
        console_error("failed to get server buffer attributes", 0);
        rewind_buffer.reset(attr.maxlength);
      } else {
        std::stringstream s;
        s << "PulseAudio: got buffer attributes: maxlength "
          << received_attr->maxlength << ", minreq " << received_attr->minreq
          << ", tlength " << received_attr->tlength << ", prebuf "
          << received_attr->prebuf;
        console::info(s.str().c_str());
        rewind_buffer.reset(received_attr->maxlength);
      }
    }

    g_pa_threaded_mainloop_unlock(mainloop);

    trigger_update.set_state(true);
  }

  static void console_error(const char* prefix, int error_code) {
    std::stringstream s;
    s << "PulseAudio: ";
    s << prefix;

    if (error_code != 0) {
      const char* error = g_pa_strerror(error_code);
      if (error != NULL) {
        s << ": " << error;
      }
    }

    console::error(s.str().c_str());
  }

  static void __stdcall stream_drained_cb(pa_stream* s, int success, void* userdata) {
    output_pulse* output = (output_pulse*)userdata;
    output->draining = false;
    output->drained = true;
    output->trigger_update.set_state(true);
  }

  static bool load_pulse_dll() {
    if (g_pa_is_loaded) return true;

    HMODULE libpulse;
    pfc::string_formatter path = core_api::get_my_full_path();
    path.truncate(path.scan_filename());
    std::wstringstream wpath_libpulse;
    wpath_libpulse << path << "pulse\\libpulse-0.dll";
    libpulse = LoadLibraryExW(wpath_libpulse.str().c_str(), NULL,
                              LOAD_WITH_ALTERED_SEARCH_PATH);
    wchar_t szFullPath[MAX_PATH] = {};
    GetCurrentDirectory(MAX_PATH, szFullPath);
    if (libpulse == NULL) {
      std::stringstream error;
      error << "Could not load libpulse-0.dll: error code " << GetLastError();
      console::error(error.str().c_str());
      return false;
    }

#define LOAD_PULSE_FUNC(x) \
do { \
    g_##x = (x)GetProcAddress(libpulse, "WIN_" #x); \
    if (!g_##x) { \
        console::error("Failed to load function " #x " from libpulse DLL!"); \
        return false; \
    } \
} while (0)

    LOAD_PULSE_FUNC(pa_strerror);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_new);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_free);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_start);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_stop);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_lock);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_unlock);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_wait);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_signal);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_accept);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_get_retval);
    LOAD_PULSE_FUNC(pa_threaded_mainloop_get_api);
    LOAD_PULSE_FUNC(pa_stream_new);
    LOAD_PULSE_FUNC(pa_stream_connect_playback);
    LOAD_PULSE_FUNC(pa_stream_disconnect);
    LOAD_PULSE_FUNC(pa_stream_unref);
    LOAD_PULSE_FUNC(pa_stream_write);
    LOAD_PULSE_FUNC(pa_stream_cancel_write);
    LOAD_PULSE_FUNC(pa_stream_drop);
    LOAD_PULSE_FUNC(pa_stream_writable_size);
    LOAD_PULSE_FUNC(pa_stream_drain);
    LOAD_PULSE_FUNC(pa_stream_set_write_callback);
    LOAD_PULSE_FUNC(pa_stream_set_state_callback);
    LOAD_PULSE_FUNC(pa_stream_set_started_callback);
    LOAD_PULSE_FUNC(pa_stream_set_underflow_callback);
    LOAD_PULSE_FUNC(pa_stream_cork);
    LOAD_PULSE_FUNC(pa_stream_is_corked);
    LOAD_PULSE_FUNC(pa_stream_flush);
    LOAD_PULSE_FUNC(pa_stream_update_sample_rate);
    LOAD_PULSE_FUNC(pa_stream_get_state);
    LOAD_PULSE_FUNC(pa_stream_get_sample_spec);
    LOAD_PULSE_FUNC(pa_stream_get_latency);
    LOAD_PULSE_FUNC(pa_stream_get_timing_info);
    LOAD_PULSE_FUNC(pa_stream_trigger);
    LOAD_PULSE_FUNC(pa_stream_update_timing_info);
    LOAD_PULSE_FUNC(pa_stream_get_buffer_attr);
    LOAD_PULSE_FUNC(pa_proplist_new);
    LOAD_PULSE_FUNC(pa_proplist_free);
    LOAD_PULSE_FUNC(pa_proplist_set);
    LOAD_PULSE_FUNC(pa_proplist_sets);
    LOAD_PULSE_FUNC(pa_proplist_setp);
    LOAD_PULSE_FUNC(pa_context_new_with_proplist);
    LOAD_PULSE_FUNC(pa_context_unref);
    LOAD_PULSE_FUNC(pa_context_errno);
    LOAD_PULSE_FUNC(pa_context_connect);
    LOAD_PULSE_FUNC(pa_context_disconnect);
    LOAD_PULSE_FUNC(pa_context_get_state);
    LOAD_PULSE_FUNC(pa_context_set_state_callback);
    LOAD_PULSE_FUNC(pa_context_set_event_callback);
    LOAD_PULSE_FUNC(pa_operation_unref);
    LOAD_PULSE_FUNC(pa_operation_get_state);
    LOAD_PULSE_FUNC(pa_bytes_to_usec);
    LOAD_PULSE_FUNC(pa_usec_to_bytes);
    LOAD_PULSE_FUNC(pa_channel_map_init_auto);
    LOAD_PULSE_FUNC(pa_stream_get_index);
    LOAD_PULSE_FUNC(pa_sw_volume_from_dB);
    LOAD_PULSE_FUNC(pa_sw_volume_to_dB);
    LOAD_PULSE_FUNC(pa_cvolume_valid);
    LOAD_PULSE_FUNC(pa_cvolume_equal);
    LOAD_PULSE_FUNC(pa_context_set_sink_input_volume);
    LOAD_PULSE_FUNC(pa_cvolume_init);
    LOAD_PULSE_FUNC(pa_cvolume_set);
    LOAD_PULSE_FUNC(pa_context_get_sink_input_info);
    LOAD_PULSE_FUNC(pa_context_subscribe);
    LOAD_PULSE_FUNC(pa_context_set_subscribe_callback);

#undef LOAD_PULSE_FUNC

    void* winelib = GetProcAddress(libpulse, "foo_out_pulse_winelib_dll");

    is_using_winelib = (winelib != NULL);
    console::info(is_using_winelib ? "PulseAudio: using winelib libpulse"
                                   : "PulseAudio: Using Windows libpulse");

    g_pa_is_loaded = true;
    return true;
  }
};

static output_factory_t<output_pulse> g_output_pulse_factory;

}  // namespace