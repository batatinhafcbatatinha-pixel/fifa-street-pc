/**
 * XAudio2 Audio Output Backend
 *
 * Provides low-latency audio output via XAudio2 (Win7+).
 * Called from the APU monitor frame to submit mixed samples.
 * Falls back gracefully if XAudio2 is unavailable.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "apu_xaudio2.h"

/* The XAudio2 backend is Windows-only. On Linux all xa2_* functions are
 * stubbed to report inactive; real audio output via SDL2 comes later. */
#if defined(_WIN32)

#define COBJMACROS
#include <windows.h>
#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "ole32.lib")

#define XA2_SAMPLE_RATE   48000
#define XA2_CHANNELS      2
#define XA2_BUF_SAMPLES   1024   /* ~21ms per submission */
#define XA2_NUM_BUFS      32     /* slots; the queue is bounded in samples */
/* Queue depth in samples. The producer (the APU frame thread, paced by the
 * host clock) and the sound card's clock drift apart and the producer has
 * scheduling jitter, so the queue is kept around XA2_TARGET by nudging the
 * playback rate (inaudible, <= 0.5%) instead of dropping or starving blocks:
 * either one is a click. Only far beyond XA2_MAX is a block dropped. */
#define XA2_TARGET        2048   /* ~43ms */
#define XA2_MAX           8192   /* ~170ms */

static IXAudio2               *g_xa2 = NULL;
static IXAudio2MasteringVoice *g_xa2_master = NULL;
static IXAudio2SourceVoice    *g_xa2_source = NULL;
static int16_t                 g_xa2_bufs[XA2_NUM_BUFS][XA2_BUF_SAMPLES][2];
static int                     g_xa2_next_buf = 0;
static int                     g_xa2_initialized = 0;
static int                     g_xa2_frames_written = 0;
static int                     g_xa2_size[XA2_NUM_BUFS];
static float                   g_xa2_avg = XA2_TARGET, g_xa2_ratio = 1.0f;
static unsigned                g_xa2_underruns, g_xa2_drops, g_xa2_submits;

int xa2_init(void)
{
    HRESULT hr;
    int com_initialized;
    WAVEFORMATEX wfx = { 0 };

    if (g_xa2_initialized) return 1;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        fprintf(stderr, "[XA2] CoInitializeEx failed: 0x%08lX\n", hr);
        return 0;
    }
    com_initialized = SUCCEEDED(hr);

    hr = XAudio2Create(&g_xa2, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr) || !g_xa2) {
        fprintf(stderr, "[XA2] XAudio2Create failed: 0x%08lX\n", hr);
        goto fail;
    }

    hr = IXAudio2_CreateMasteringVoice(g_xa2, &g_xa2_master,
        XA2_CHANNELS, XA2_SAMPLE_RATE, 0, NULL, NULL, 0);
    if (FAILED(hr)) {
        fprintf(stderr, "[XA2] CreateMasteringVoice failed: 0x%08lX\n", hr);
        goto fail;
    }

    wfx.wFormatTag      = WAVE_FORMAT_PCM;
    wfx.nChannels       = XA2_CHANNELS;
    wfx.nSamplesPerSec  = XA2_SAMPLE_RATE;
    wfx.wBitsPerSample  = 16;
    wfx.nBlockAlign     = XA2_CHANNELS * 2;
    wfx.nAvgBytesPerSec = XA2_SAMPLE_RATE * wfx.nBlockAlign;

    hr = IXAudio2_CreateSourceVoice(g_xa2, &g_xa2_source,
        &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, NULL, NULL, NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "[XA2] CreateSourceVoice failed: 0x%08lX\n", hr);
        goto fail;
    }

    hr = IXAudio2SourceVoice_Start(g_xa2_source, 0, XAUDIO2_COMMIT_NOW);
    if (FAILED(hr)) {
        fprintf(stderr, "[XA2] Start failed: 0x%08lX\n", hr);
        goto fail;
    }

    g_xa2_next_buf = 0;
    g_xa2_initialized = 1;
    g_xa2_frames_written = 0;

    fprintf(stderr, "[XA2] XAudio2 initialized (%d Hz stereo 16-bit, %d x %d-sample buffers)\n",
            XA2_SAMPLE_RATE, XA2_NUM_BUFS, XA2_BUF_SAMPLES);
    return 1;

fail:
    xa2_shutdown();
    /* Failed initialization still runs on the COM-initializing thread. */
    if (com_initialized) CoUninitialize();
    return 0;
}

void xa2_shutdown(void)
{
    if (g_xa2_source) {
        IXAudio2SourceVoice_Stop(g_xa2_source, 0, XAUDIO2_COMMIT_NOW);
        IXAudio2SourceVoice_FlushSourceBuffers(g_xa2_source);
        g_xa2_source->lpVtbl->DestroyVoice(g_xa2_source);
        g_xa2_source = NULL;
    }
    if (g_xa2_master) {
        g_xa2_master->lpVtbl->DestroyVoice(g_xa2_master);
        g_xa2_master = NULL;
    }
    if (g_xa2) {
        IXAudio2_Release(g_xa2);
        g_xa2 = NULL;
    }

    if (g_xa2_initialized)
        fprintf(stderr, "[XA2] Shut down (%d frames written)\n", g_xa2_frames_written);
    g_xa2_initialized = 0;
}

int xa2_is_active(void)
{
    return g_xa2_initialized;
}

/* Submit a buffer of mixed samples to XAudio2.
 * Called from APU frame thread. Returns 1 if buffer was submitted. */
static int xa2_queue(const int16_t *samples, int n)
{
    XAUDIO2_BUFFER xbuf;
    int idx = g_xa2_next_buf;
    if (samples)
        memcpy(g_xa2_bufs[idx], samples, (size_t)n * XA2_CHANNELS * sizeof(int16_t));
    else
        memset(g_xa2_bufs[idx], 0, (size_t)n * XA2_CHANNELS * sizeof(int16_t));
    memset(&xbuf, 0, sizeof(xbuf));
    xbuf.AudioBytes = (UINT32)(n * XA2_CHANNELS * (int)sizeof(int16_t));
    xbuf.pAudioData = (const BYTE *)g_xa2_bufs[idx];
    if (FAILED(IXAudio2SourceVoice_SubmitSourceBuffer(g_xa2_source, &xbuf, NULL)))
        return 0;
    g_xa2_size[idx] = n;
    g_xa2_next_buf = (idx + 1) % XA2_NUM_BUFS;
    return 1;
}

int xa2_submit_samples(const int16_t *samples, int num_samples)
{
    XAUDIO2_VOICE_STATE state;
    int copy_samples, queued = 0;
    float ratio;

    if (!g_xa2_initialized || !g_xa2_source) return 0;

    IXAudio2SourceVoice_GetState(g_xa2_source, &state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    if (state.BuffersQueued >= XA2_NUM_BUFS - 2) {
        g_xa2_drops++;
        return 0;
    }
    /* Samples still queued: the newest BuffersQueued slots. */
    for (UINT32 i = 1; i <= state.BuffersQueued; i++)
        queued += g_xa2_size[(g_xa2_next_buf + XA2_NUM_BUFS - (int)i) % XA2_NUM_BUFS];
    if (queued > XA2_MAX) {
        g_xa2_drops++;
        return 0;
    }
    if (state.BuffersQueued == 0 && g_xa2_frames_written) {
        /* Starved: the gap already happened; start again with a cushion so
         * it does not chatter. */
        g_xa2_underruns++;
        xa2_queue(NULL, XA2_TARGET / 2);
        queued = XA2_TARGET / 2;
    }

    copy_samples = (num_samples > XA2_BUF_SAMPLES) ? XA2_BUF_SAMPLES : num_samples;
    if (!xa2_queue(samples, copy_samples)) return 0;
    g_xa2_frames_written++;

    /* Rate nudge from the smoothed depth: fuller than the target plays a
     * little faster, emptier a little slower. */
    g_xa2_avg += ((float)queued - g_xa2_avg) * 0.02f;
    ratio = 1.0f + 0.005f * (g_xa2_avg - XA2_TARGET) / XA2_TARGET;
    ratio = ratio < 0.995f ? 0.995f : ratio > 1.005f ? 1.005f : ratio;
    if (fabsf(ratio - g_xa2_ratio) > 0.0001f) {
        g_xa2_ratio = ratio;
        IXAudio2SourceVoice_SetFrequencyRatio(g_xa2_source, ratio, XAUDIO2_COMMIT_NOW);
    }
    if (++g_xa2_submits % 2000 == 0)
        fprintf(stderr, "[XA2] queue %.0f samples (target %d), rate %.4f, underruns %u, drops %u\n",
                g_xa2_avg, XA2_TARGET, g_xa2_ratio, g_xa2_underruns, g_xa2_drops);
    return 1;
}

int xa2_get_buffer_size(void)
{
    return XA2_BUF_SAMPLES;
}

/* Output volume, 0..1, on the mastering voice: the in-game options' volume
 * and the silence while the options pause the game. */
void xa2_set_volume(float volume)
{
    if (g_xa2_master)
        IXAudio2MasteringVoice_SetVolume(g_xa2_master, volume, XAUDIO2_COMMIT_NOW);
}

#else /* !_WIN32 -- SDL2 audio, the same queue and rate nudge as above */

/* The APU frame thread writes into a ring; SDL's callback reads it at a
 * fractional step (the rate nudge, <= 0.5%, linear interpolation) so the
 * ring stays near SDL_TARGET samples deep without dropping or starving
 * blocks -- the role SetFrequencyRatio plays for XAudio2. */
#include <SDL2/SDL.h>

#define SDL_RATE    48000
#define SDL_TARGET  2048
#define SDL_MAX     8192
#define SDL_RING    16384                    /* stereo frames, power of two */

static SDL_AudioDeviceID s_dev;
static int16_t s_ring[SDL_RING][2];
static volatile uint64_t s_w;                /* frames written (producer) */
static double s_r;                           /* read position (callback) */
static float s_ratio = 1.0f, s_avg = SDL_TARGET, s_volume = 1.0f;
static unsigned s_underruns, s_drops, s_submits;
static int s_started;

static void sdl_callback(void *ud, Uint8 *stream, int len)
{
    int16_t *out = (int16_t *)stream;
    int n = len / 4, i;
    uint64_t w = __atomic_load_n(&s_w, __ATOMIC_ACQUIRE);
    float vol = s_volume, step = s_ratio;
    (void)ud;
    for (i = 0; i < n; i++) {
        uint64_t k = (uint64_t)s_r;
        float f = (float)(s_r - (double)k);
        if (k + 1 >= w) {                        /* starved */
            if (s_started)
                s_underruns++;
            for (; i < n; i++)
                out[2 * i] = out[2 * i + 1] = 0;
            break;
        }
        for (int c = 0; c < 2; c++) {
            float a = s_ring[k & (SDL_RING - 1)][c], b = s_ring[(k + 1) & (SDL_RING - 1)][c];
            float v = (a + (b - a) * f) * vol;
            out[2 * i + c] = (int16_t)(v > 32767.0f ? 32767 : v < -32768.0f ? -32768 : v);
        }
        s_r += step;
    }
}

int xa2_init(void)
{
    SDL_AudioSpec want, have;
    if (s_dev)
        return 1;
    if (!SDL_WasInit(SDL_INIT_AUDIO) && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "[XA2] SDL audio init failed: %s\n", SDL_GetError());
        return 0;
    }
    SDL_zero(want);
    want.freq = SDL_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 512;
    want.callback = sdl_callback;
    s_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (!s_dev) {
        fprintf(stderr, "[XA2] SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return 0;
    }
    SDL_PauseAudioDevice(s_dev, 0);
    fprintf(stderr, "[XA2] SDL audio on \"%s\" (%d Hz stereo 16-bit, %d-sample periods)\n",
            SDL_GetCurrentAudioDriver(), have.freq, have.samples);
    return 1;
}

void xa2_shutdown(void)
{
    if (s_dev) {
        SDL_CloseAudioDevice(s_dev);
        s_dev = 0;
    }
}

int xa2_is_active(void)
{
    return s_dev != 0;
}

int xa2_submit_samples(const int16_t *samples, int num_samples)
{
    uint64_t w, r;
    int queued, i;
    float ratio;
    if (!s_dev || num_samples <= 0)
        return 0;
    SDL_LockAudioDevice(s_dev);
    w = s_w;
    r = (uint64_t)s_r;
    queued = (int)(w > r ? w - r : 0);
    if (queued > SDL_MAX || queued + num_samples >= SDL_RING) {
        s_drops++;
        SDL_UnlockAudioDevice(s_dev);
        return 0;
    }
    if (queued <= 1 && s_started) {
        /* Starved: restart with a cushion of silence so it does not chatter. */
        s_r = (double)w;
        for (i = 0; i < SDL_TARGET / 2; i++, w++)
            s_ring[w & (SDL_RING - 1)][0] = s_ring[w & (SDL_RING - 1)][1] = 0;
        queued = SDL_TARGET / 2;
    }
    for (i = 0; i < num_samples; i++, w++) {
        s_ring[w & (SDL_RING - 1)][0] = samples[2 * i];
        s_ring[w & (SDL_RING - 1)][1] = samples[2 * i + 1];
    }
    __atomic_store_n(&s_w, w, __ATOMIC_RELEASE);
    s_started = 1;
    s_avg += ((float)queued - s_avg) * 0.02f;
    ratio = 1.0f + 0.005f * (s_avg - SDL_TARGET) / SDL_TARGET;
    s_ratio = ratio < 0.995f ? 0.995f : ratio > 1.005f ? 1.005f : ratio;
    SDL_UnlockAudioDevice(s_dev);
    if (++s_submits % 2000 == 0)
        fprintf(stderr, "[XA2] queue %.0f samples (target %d), rate %.4f, underruns %u, drops %u\n",
                s_avg, SDL_TARGET, s_ratio, s_underruns, s_drops);
    return 1;
}

int xa2_get_buffer_size(void)
{
    return 1024;
}

void xa2_set_volume(float v)
{
    s_volume = v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}

#endif /* _WIN32 */
