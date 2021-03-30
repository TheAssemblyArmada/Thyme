/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stub library containing subset of functions from mss32.dll as used by the SAGE engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include <stdint.h>

#if !defined _MSC_VER
#if !defined(__stdcall)
#if defined __has_attribute && __has_attribute(stdcall)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif /* !defined __stdcall */
#endif /* !defined COMPILER_MSVC */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These will have been structs in the real SDK headers, but are only accessed through pointers
 * so we don't care unless access to the internals is required.
 */
typedef void *H3DPOBJECT;
typedef void *H3DSAMPLE;
typedef void *HSAMPLE;
typedef void *HSTREAM;
typedef void *HDIGDRIVER;
typedef void *HAUDIO;
typedef void *AILLPDIRECTSOUND;
typedef void *AILLPDIRECTSOUNDBUFFER;
typedef void *HMDIDRIVER;
typedef void *HDLSDEVICE;

typedef struct _AILSOUNDINFO
{
    int32_t format;
    const void *data_ptr;
    uint32_t data_len;
    uint32_t rate;
    int32_t bits;
    int32_t channels;
    uint32_t samples;
    uint32_t block_size;
    const void *initial_ptr;
} AILSOUNDINFO;

/*
 * Various callback typedefs.
 */
typedef int(__stdcall *AIL_file_open_callback)(const char *, uintptr_t *);
typedef void(__stdcall *AIL_file_close_callback)(uintptr_t);
typedef int32_t(__stdcall *AIL_file_seek_callback)(uintptr_t, int32_t, uint32_t);
typedef uint32_t(__stdcall *AIL_file_read_callback)(uintptr_t, void *, uint32_t);
typedef void(__stdcall *AIL_stream_callback)(void *);
typedef void(__stdcall *AIL_3dsample_callback)(void*);
typedef void(__stdcall *AIL_sample_callback)(void *);

void __stdcall AIL_set_3D_user_data(H3DPOBJECT obj, uint32_t index, int32_t value);
void __stdcall AIL_sample_volume_pan(HSAMPLE sample, float *volume, float *pan);
void __stdcall AIL_set_filter_sample_preference(HSAMPLE sample, const char *name, const void *val);
void __stdcall AIL_shutdown(void);
void __stdcall AIL_set_file_callbacks(AIL_file_open_callback opencb, AIL_file_close_callback closecb,
    AIL_file_seek_callback seekcb, AIL_file_read_callback readcb);
void __stdcall AIL_pause_stream(HSTREAM stream, int32_t onoff);
AIL_stream_callback __stdcall AIL_register_stream_callback(HSTREAM stream, AIL_stream_callback callback);
void __stdcall AIL_stop_3D_sample(H3DSAMPLE sample);
AIL_3dsample_callback __stdcall AIL_register_3D_EOS_callback(H3DSAMPLE sample, AIL_3dsample_callback EOS);
void __stdcall AIL_stop_sample(HSAMPLE sample);
AIL_sample_callback __stdcall AIL_register_EOS_callback(HSAMPLE sample, AIL_sample_callback EOS);
void __stdcall AIL_resume_3D_sample(H3DSAMPLE sample);
void __stdcall AIL_resume_sample(HSAMPLE sample);
void __stdcall AIL_start_stream(HSTREAM stream);
void __stdcall AIL_set_stream_loop_count(HSTREAM stream, int32_t count);
void __stdcall AIL_set_stream_volume_pan(HSTREAM stream, float volume, float pan);
HSTREAM __stdcall AIL_open_stream(HDIGDRIVER dig, const char *filename, int32_t stream_mem);
void __stdcall AIL_start_sample(HSAMPLE sample);
int32_t __stdcall AIL_set_sample_file(HSAMPLE sample, const void *file_image, int32_t block);
void __stdcall AIL_init_sample(HSAMPLE sample);
void __stdcall AIL_close_stream(HSTREAM stream);
void __stdcall AIL_quick_unload(HAUDIO audio);
void __stdcall AIL_release_3D_sample_handle(H3DSAMPLE sample);
void __stdcall AIL_release_sample_handle(HSAMPLE sample);
void __stdcall AIL_stream_volume_pan(HSTREAM stream, float *volume, float *pan);
void __stdcall AIL_set_3D_sample_volume(H3DSAMPLE sample, float volume);
int32_t __stdcall AIL_WAV_info(const void *data, AILSOUNDINFO *info);
int32_t __stdcall AIL_decompress_ADPCM(const AILSOUNDINFO *info, void **outdata, uint32_t *outsize);
void __stdcall AIL_mem_free_lock(void *ptr);
HAUDIO __stdcall AIL_quick_load_and_play(const char *filename, uint32_t loop_count, int32_t wait_request);
void __stdcall AIL_quick_set_volume(HAUDIO audio, float volume, float extravol);
HSAMPLE __stdcall AIL_allocate_sample_handle(HDIGDRIVER dig);
void __stdcall AIL_set_sample_user_data(HSAMPLE sample, uint32_t index, int32_t value);
void __stdcall AIL_set_sample_volume_pan(HSAMPLE sample, float volume, float pan);
H3DSAMPLE __stdcall AIL_allocate_3D_sample_handle(uint32_t lib);
int32_t __stdcall AIL_set_3D_sample_file(H3DSAMPLE sample, const void *file_image);
void __stdcall AIL_set_3D_sample_distances(H3DSAMPLE sample, float max_dist, float min_dist);
void __stdcall AIL_start_3D_sample(H3DSAMPLE sample);
uint32_t __stdcall AIL_set_sample_processor(HSAMPLE sample, int32_t pipeline_stage, uint32_t provider);
void __stdcall AIL_set_sample_playback_rate(HSAMPLE sample, int32_t playback_rate);
int32_t __stdcall AIL_sample_playback_rate(HSAMPLE sample);
void __stdcall AIL_set_3D_sample_occlusion(H3DSAMPLE sample, float occlusion);
void __stdcall AIL_set_3D_orientation(
    H3DPOBJECT obj, float X_face, float Y_face, float Z_face, float X_up, float Y_up, float Z_up);
void __stdcall AIL_stream_ms_position(HSTREAM sample, int32_t *total_milliseconds, int32_t *current_milliseconds);
void __stdcall AIL_set_3D_position(H3DPOBJECT obj, float X, float Y, float Z);
void __stdcall AIL_set_3D_speaker_type(uint32_t lib, int32_t speaker_type);
void __stdcall AIL_close_3D_listener(H3DPOBJECT listener);
void __stdcall AIL_close_3D_provider(uint32_t lib);
void __stdcall AIL_get_DirectSound_info(HSAMPLE sample, AILLPDIRECTSOUND *lplpDS, AILLPDIRECTSOUNDBUFFER *lplpDSB);
int32_t __stdcall AIL_open_3D_provider(uint32_t lib);
H3DPOBJECT __stdcall AIL_open_3D_listener(uint32_t lib);
char *__stdcall AIL_set_redist_directory(const char *dir);
int32_t __stdcall AIL_startup(void);
int32_t __stdcall AIL_quick_startup(
    int32_t use_digital, int32_t use_MIDI, uint32_t output_rate, int32_t output_bits, int32_t output_channels);
void __stdcall AIL_quick_handles(HDIGDRIVER *pdig, HMDIDRIVER *pmdi, HDLSDEVICE *pdls);
int32_t __stdcall AIL_enumerate_3D_providers(uint32_t *next, uint32_t *dest, char **name);
int32_t __stdcall AIL_enumerate_filters(uint32_t *next, uint32_t *dest, char **name);
int32_t __stdcall AIL_stream_loop_count(HSTREAM stream);
int32_t __stdcall AIL_3D_sample_playback_rate(H3DSAMPLE sample);
void __stdcall AIL_set_3D_sample_playback_rate(H3DSAMPLE sample, int32_t playback_rate);

#ifdef __cplusplus
} // extern "C"
#endif
