/*
 * Native DirectSound (RECOMP_DS_NATIVE=1).
 *
 * The XDK DirectSound library programs the MCPX voice processor through its
 * registers; the runtime then reimplements that hardware in C. Here the 23
 * library entry points FIFA Street calls are implemented directly, with a
 * voice mixer of their own, a speaker fold and a 48 kHz output thread that
 * feeds the host's audio (XAudio2, SDL off Windows) directly. Neither the
 * library nor any model of the APU (registers, voice processor, DSPs) runs.
 *
 * What the title uses (measured, RECOMP_DS_LOG; conversions read from the
 * library with REA/Ghidra):
 *   - 176 mono sound-effect buffers, Xbox ADPCM (tag 0x69) or PCM16, played
 *     one shot or looped from game memory (SetBufferData), 6 mixbins each;
 *   - one 6-channel PCM16 buffer looping 0.1 s, refilled by EA's software
 *     mixer by GetCurrentPosition (music, crowd, commentary);
 *   - SetEG is always the "multi" EG with zero pitch/filter modulation and
 *     SetFilter always DLS2 with FC 0 / FC1 0xFFFF; both are honoured anyway.
 * Conversions (library 0x269D3A, 0x2660D7, 0x26A5F2, 0x26A784):
 *   volume register = min(0xFFF, -(mixbin volume + buffer base) * 64 / 100),
 *     base -600 (the default 6 dB headroom), gain 10^(-reg / 1280);
 *   pitch = round(4096 * log2(Hz / 48000)), step 2^(pitch / 4096);
 *   filter DLS2: FC0 = coefficient 0, FC1 = coefficient 1 (state-variable LP,
 *     fc = 2^(FC0/4096), q = FC1/0x8000), as the voice processor runs it.
 * Effects (DownloadEffectsImage / SetEffectData / I3DL2) are accepted and not
 * processed -- RECOMP_AUDIO_NATIVE never ran the GP effects either.
 */
#include "hle_win32.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"

enum {
#define HLE_DS(va, name, conv, args) DS_##name,
#include "hle_dsound_table.h"
#undef HLE_DS
    DS_COUNT
};

#define NUM_MIXBINS 32
#define FRAME 32
#define MAX_BUFS 512
#define BUF_MAGIC 0x4253534Eu                /* 'NSSB' at the handle */
#define DS_MAGIC  0x4453534Eu

extern uint32_t xbox_HeapAlloc(uint32_t size, uint32_t alignment);

typedef struct { float f, q, qnrm, h, b, l, p, n; } Svf;

typedef struct {
    uint32_t handle;
    int used, refs;
    /* format */
    int adpcm, channels, bits, align, rate;
    /* data */
    uint32_t data, bytes, own;
    uint32_t loop_start, loop_len;            /* bytes */
    /* playback */
    double pos;                               /* in frames */
    int playing, looping, paused;
    int pitch;
    /* mix */
    int nbins;
    uint8_t bin[8];
    int32_t binvol[NUM_MIXBINS];
    int fmode;
    int16_t fc0;
    uint16_t fc1;
    Svf svf[6];
    /* ADPCM cache */
    int cached_block;
    int16_t dec[65 * 2 * 2];
    uint64_t frames_played;
} DsBuf;

static CRITICAL_SECTION s_lock;
static DsBuf s_buf[MAX_BUFS];
static uint32_t s_ds_handle;
static int s_headroom[NUM_MIXBINS];
static int s_started;
static uint64_t s_calls[DS_COUNT], s_mix_frames;

static int s_shadow = -1;                     /* RECOMP_DS_SHADOW: library plays, native compared */

int hle_ds_native_enabled(void)
{
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("RECOMP_DS_NATIVE"), *sh = getenv("RECOMP_DS_SHADOW");
        on = e && *e && *e != '0';
        s_shadow = !on && sh && *sh && *sh != '0';
        if (on || s_shadow)
            InitializeCriticalSection(&s_lock);
    }
    return on;
}

/* ------------------------------------------------------------ helpers */

static DsBuf *buf_of(uint32_t handle)
{
    uint32_t i;
    if (!handle || MEM32(handle) != BUF_MAGIC)
        return NULL;
    i = MEM32(handle + 4);
    return i < MAX_BUFS && s_buf[i].used && s_buf[i].handle == handle ? &s_buf[i] : NULL;
}

static uint32_t frames_of_bytes(const DsBuf *b, uint32_t bytes)
{
    if (b->adpcm)
        return bytes / (36u * (uint32_t)b->channels) * 64u;
    return b->align ? bytes / (uint32_t)b->align : 0;
}

static uint32_t bytes_of_frames(const DsBuf *b, uint32_t frames)
{
    if (b->adpcm)
        return frames / 64u * 36u * (uint32_t)b->channels;
    return frames * (uint32_t)b->align;
}

static int pitch_of(uint32_t hz)
{
    if (hz == 48000 || !hz)
        return 0;
    return (int)lrint(4096.0 * log2((double)(float)((float)hz * 2.0833333e-05f)));
}

/* Library 0x269D3A: mB -> voice volume register -> linear gain. */
static float gain_of(int32_t mb)
{
    uint32_t reg = (uint32_t)(-(mb - 600)) * 64u / 100u;
    if (reg > 0xFFF)
        reg = 0xFFF;
    return reg == 0xFFF ? 0.0f : powf(10.0f, (float)reg / (64.0f * -20.0f));
}

static void svf_setup(Svf *s, float fc, float q)
{
    s->f = fc;
    s->q = q;
    s->qnrm = (float)sqrt(q / 2.0 + 0.01);
}

static float ftz(float x) { return fabsf(x) < 1e-30f ? 0.0f : x; }

static float svf_run(Svf *s, float in)       /* the voice processor's SVF, LP output */
{
    in = s->qnrm * in;
    s->b = ftz(s->b - s->b * s->b * s->b * 0.001f);
    s->h = ftz(in - s->l - s->q * s->b);
    s->b = s->b + s->f * s->h;
    s->l = ftz(s->l + s->f * s->b);
    s->n = s->l + s->h;
    s->p = s->l - s->h;
    return s->l;
}

/* Xbox ADPCM block (IMA, 36 bytes per channel): 65 samples per channel,
 * the voice processor plays the first 64. Same decoder as apu_state.h. */
static void adpcm_block(int16_t *out, const uint8_t *in, int channels)
{
    static const uint16_t steps[89] = {
        7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,
        34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,
        157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,
        724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,
        3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,
        15289,16818,18500,20350,22385,24623,27086,29794,32767 };
    static const int idx_tab[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
    int32_t pcm[2];
    int index[2], ch, chunks, i, k;
    int16_t *o = out;
    for (ch = 0; ch < channels; ch++) {
        *o++ = (int16_t)(pcm[ch] = (int16_t)(in[0] | (in[1] << 8)));
        index[ch] = in[2] > 88 ? 88 : in[2];
        in += 4;
    }
    for (chunks = 8; chunks--;) {
        for (ch = 0; ch < channels; ch++) {
            for (i = 0; i < 4; i++) {
                for (k = 0; k < 2; k++) {
                    int nib = k ? (*in >> 4) & 0xF : *in & 0xF;
                    int step = steps[index[ch]], delta = step >> 3;
                    if (nib & 1) delta += step >> 2;
                    if (nib & 2) delta += step >> 1;
                    if (nib & 4) delta += step;
                    if (nib & 8) delta = -delta;
                    pcm[ch] += delta;
                    if (pcm[ch] > 32767) pcm[ch] = 32767;
                    if (pcm[ch] < -32768) pcm[ch] = -32768;
                    index[ch] += idx_tab[nib & 7];
                    if (index[ch] < 0) index[ch] = 0;
                    if (index[ch] > 88) index[ch] = 88;
                    o[(i * 2 + k) * channels] = (int16_t)pcm[ch];
                }
                in++;
            }
            o++;
        }
        o += channels * 7;
    }
}

/* One frame (all channels) as floats -1..1. */
static void fetch(DsBuf *b, uint32_t frame, float *out)
{
    int c;
    if (b->adpcm) {
        uint32_t block = frame / 64u, at = frame % 64u;
        if ((int)block != b->cached_block) {
            adpcm_block(b->dec, (const uint8_t *)XBOX_PTR(b->data + block * 36u * (uint32_t)b->channels),
                        b->channels);
            b->cached_block = (int)block;
        }
        for (c = 0; c < b->channels; c++)
            out[c] = (float)b->dec[at * (uint32_t)b->channels + (uint32_t)c] / 32768.0f;
        return;
    }
    for (c = 0; c < b->channels; c++) {
        uint32_t a = b->data + frame * (uint32_t)b->align;
        if (b->bits == 8)
            out[c] = ((float)MEM8(a + (uint32_t)c) - 128.0f) / 128.0f;
        else
            out[c] = (float)(int16_t)MEM16(a + 2u * (uint32_t)c) / 32768.0f;
    }
}

/* Library 0x26B6F4: play cursor, and write cursor = play + max(32 samples,
 * the buffer's minimum gap), wrapped in the loop region or the buffer. */
static void cursors(const DsBuf *b, uint32_t *play_out, uint32_t *write_out)
{
    uint32_t play = bytes_of_frames(b, (uint32_t)b->pos), gap, write;
    if (b->playing) {
        gap = b->adpcm ? 36u * (uint32_t)b->channels : 0u;    /* measured against the library (shadow) */
        if (gap < bytes_of_frames(b, 32))
            gap = bytes_of_frames(b, 32);
        write = play + gap;
        if (b->loop_len && play >= b->loop_start && play < b->loop_start + b->loop_len)
            write = b->loop_start + (write - b->loop_start) % b->loop_len;
        else if (b->bytes)
            write %= b->bytes;
    } else {
        write = play;
    }
    *play_out = play;
    *write_out = write;
}

/* ------------------------------------------------------------- mixer */

static void mix_buffer(DsBuf *b, float mixbins[NUM_MIXBINS][FRAME])
{
    float out[FRAME][6], s0[6], s1[6];
    uint32_t total = frames_of_bytes(b, b->bytes), ls, le;
    double step = pow(2.0, b->pitch / 4096.0);
    int i, c, j, n = 0;
    if (!b->data || !total) {
        b->playing = 0;
        return;
    }
    ls = b->loop_len ? frames_of_bytes(b, b->loop_start) : 0;
    le = b->loop_len ? ls + frames_of_bytes(b, b->loop_len) : total;
    if (le > total) le = total;
    if (ls >= le) ls = 0;
    for (i = 0; i < FRAME; i++) {
        uint32_t i0 = (uint32_t)b->pos, i1 = i0 + 1;
        float fr = (float)(b->pos - (double)i0);
        if (b->looping) {
            if (i1 >= le) i1 = ls;
        } else if (i1 >= total) {
            i1 = i0;
        }
        fetch(b, i0, s0);
        fetch(b, i1, s1);
        for (c = 0; c < b->channels; c++)
            out[i][c] = s0[c] + (s1[c] - s0[c]) * fr;
        n++;
        b->pos += step;
        if (b->looping) {
            while (b->pos >= (double)le)
                b->pos -= (double)(le - ls);
        } else if (b->pos >= (double)total) {
            b->pos = (double)total;
            b->playing = 0;
            break;
        }
    }
    b->frames_played += (uint64_t)n;
    if (b->fmode == 1) {
        float fc = powf(2.0f, b->fc0 / 4096.0f), q = b->fc1 / (float)0x8000;
        if (fc < 0.003906f) fc = 0.003906f;
        if (fc > 1.0f) fc = 1.0f;
        if (q < 0.079407f) q = 0.079407f;
        if (q > 1.0f) q = 1.0f;
        for (c = 0; c < b->channels; c++) {
            svf_setup(&b->svf[c], fc, q);
            for (i = 0; i < n; i++) {
                float v = svf_run(&b->svf[c], out[i][c]);
                out[i][c] = v > 1.0f ? 1.0f : v < -1.0f ? -1.0f : v;
            }
        }
    }
    for (j = 0; j < b->nbins; j++) {
        int bin = b->bin[j], ch = b->channels == 1 ? 0 : j % b->channels;
        float g = gain_of(b->binvol[bin]) / (float)(1 << (s_headroom[bin] & 7));
        if (g == 0.0f)
            continue;
        for (i = 0; i < n; i++)
            mixbins[bin][i] += g * out[i][ch];
    }
}

static void mix_voices(float mixbins[NUM_MIXBINS][FRAME])
{
    int i;
    EnterCriticalSection(&s_lock);
    for (i = 0; i < MAX_BUFS; i++)
        if (s_buf[i].used && s_buf[i].playing && !s_buf[i].paused)
            mix_buffer(&s_buf[i], mixbins);
    s_mix_frames++;
    LeaveCriticalSection(&s_lock);
}

/* -------------------------------------------------------- API objects */

static uint32_t alloc_handle(uint32_t magic, uint32_t index)
{
    uint32_t h = xbox_HeapAlloc(64, 16);
    if (h) {
        memset((void *)XBOX_PTR(h), 0, 64);
        MEM32(h) = magic;
        MEM32(h + 4) = index;
    }
    return h;
}

static void set_mixbins(DsBuf *b, uint32_t list)
{
    uint32_t cnt, p, k;
    if (!list || !(cnt = MEM32(list)) || !(p = MEM32(list + 4))) {
        b->nbins = b->channels < 8 ? b->channels : 8;         /* channel i -> speaker bin i, 0 dB */
        for (k = 0; k < (uint32_t)b->nbins; k++) {
            b->bin[k] = (uint8_t)k;
            b->binvol[k] = 0;
        }
        return;
    }
    if (cnt > 8)
        cnt = 8;
    b->nbins = (int)cnt;
    for (k = 0; k < cnt; k++) {
        uint32_t bin = MEM32(p + 8 * k) & (NUM_MIXBINS - 1);
        b->bin[k] = (uint8_t)bin;
        b->binvol[bin] = (int32_t)MEM32(p + 8 * k + 4);
    }
}

static uint32_t create_buffer(uint32_t desc, uint32_t out)
{
    uint32_t wfx = MEM32(desc + 12), i;
    DsBuf *b = NULL;
    for (i = 0; i < MAX_BUFS; i++)
        if (!s_buf[i].used) {
            b = &s_buf[i];
            break;
        }
    if (!b || !wfx)
        return 0x8007000Eu;                                    /* E_OUTOFMEMORY */
    memset(b, 0, sizeof *b);
    b->handle = alloc_handle(BUF_MAGIC, i);
    if (!b->handle)
        return 0x8007000Eu;
    b->used = 1;
    b->refs = 1;
    b->adpcm = MEM16(wfx) == 0x69;
    b->channels = MEM16(wfx + 2) ? MEM16(wfx + 2) : 1;
    if (b->channels > 6)
        b->channels = 6;
    b->rate = (int)MEM32(wfx + 4);
    b->align = MEM16(wfx + 12);
    b->bits = MEM16(wfx + 14);
    b->pitch = pitch_of((uint32_t)b->rate);
    b->cached_block = -1;
    b->bytes = MEM32(desc + 8);
    if (b->bytes) {
        b->own = b->data = xbox_HeapAlloc(b->bytes, 16);
        if (b->data)
            memset((void *)XBOX_PTR(b->data), b->bits == 8 ? 0x80 : 0, b->bytes);
    }
    set_mixbins(b, MEM32(desc + 16));
    if (out)
        MEM32(out) = b->handle;
    return 0;
}

/* ------------------------------------------------------- host output */

/* The speaker mix: the six speaker mixbins (FL, FR, C, LFE, BL, BR) folded to
 * stereo with the ITU coefficients, LFE dropped -- what the GP/EP programs
 * did for this title -- under a peak limiter (instant attack, ~60 ms
 * release), since crowd + commentary + music can sum past full scale. */
static float s_limit_gain = 1.0f;

static void fold_to_stereo(float mixbins[NUM_MIXBINS][FRAME], int16_t (*out)[2])
{
    int i;
    for (i = 0; i < FRAME; i++) {
        float c = 0.70710678f * mixbins[2][i];
        float l = mixbins[0][i] + c + 0.70710678f * mixbins[4][i];
        float r = mixbins[1][i] + c + 0.70710678f * mixbins[5][i];
        float peak = fabsf(l) > fabsf(r) ? fabsf(l) : fabsf(r);
        if (peak * s_limit_gain > 0.98f)
            s_limit_gain = 0.98f / peak;
        else
            s_limit_gain += (1.0f - s_limit_gain) * 0.00035f;
        l *= s_limit_gain;
        r *= s_limit_gain;
        l = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
        r = r > 1.0f ? 1.0f : r < -1.0f ? -1.0f : r;
        out[i][0] = (int16_t)(l * 32767.0f);
        out[i][1] = (int16_t)(r * 32767.0f);
    }
}

extern int xa2_init(void);
extern int xa2_submit_samples(const int16_t *samples, int num_samples);
extern volatile int g_audio_muted;

#define OUT_FRAMES 8                          /* 256 samples, 5.33 ms per block */

/* 48 kHz on the host clock: a block of 8 mixer frames every 256 samples,
 * resynchronised if the thread falls far behind. The output queue (XAudio2,
 * or SDL off Windows) absorbs the jitter and trims the drift with its rate
 * nudge. RECOMP_NATIVE_PCM=<file> captures 60 s of the output
 * (RECOMP_PCM_FROM=<s> to start later). */
static DWORD WINAPI audio_out_thread(LPVOID unused)
{
    LARGE_INTEGER f, now;
    long long period, next;
    unsigned long long blocks = 0, from = 0;
    FILE *pcm = NULL;
    const char *path = getenv("RECOMP_NATIVE_PCM"), *e = getenv("RECOMP_PCM_FROM");
    (void)unused;
    if (!xa2_init())
        fprintf(stderr, "[DS-NATIVE] host audio output unavailable; mixing silently\n");
    if (path && *path)
        pcm = fopen(path, "wb");
    if (e)
        from = (unsigned long long)atol(e) * 48000ull / 256ull;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
    QueryPerformanceFrequency(&f);
    period = f.QuadPart * 256 / 48000;
    QueryPerformanceCounter(&now);
    next = now.QuadPart;
    for (;;) {
        int16_t block[OUT_FRAMES * FRAME][2];
        int k;
        for (k = 0; k < OUT_FRAMES; k++) {
            float mixbins[NUM_MIXBINS][FRAME];
            memset(mixbins, 0, sizeof mixbins);
            mix_voices(mixbins);
            fold_to_stereo(mixbins, block + k * FRAME);
        }
        if (g_audio_muted)
            memset(block, 0, sizeof block);
        if (pcm && blocks >= from && blocks < from + 60ull * 48000ull / 256ull) {
            fwrite(block, 1, sizeof block, pcm);
            fflush(pcm);
        }
        blocks++;
        xa2_submit_samples(&block[0][0], OUT_FRAMES * FRAME);
        next += period;
        QueryPerformanceCounter(&now);
        if (now.QuadPart - next > 8 * period)
            next = now.QuadPart;
        for (;;) {
            long long left;
            QueryPerformanceCounter(&now);
            left = next - now.QuadPart;
            if (left <= 0)
                break;
            left = left * 1000 / f.QuadPart;
            Sleep(left > 1 ? (DWORD)(left - 1) : 1);
        }
    }
    return 0;
}

static void start_mixer(void)
{
    if (!s_started) {
        HANDLE th;
        s_started = 1;
        th = CreateThread(NULL, 0, audio_out_thread, NULL, 0, NULL);
        if (th)
            CloseHandle(th);
        fprintf(stderr, "[DS-NATIVE] mixer and host output started (no APU)\n");
    }
}

/* ----------------------------------------------------------- the API */

static int ds_do(int id, const uint32_t *a, uint32_t *ret);

/* Returns 1 when handled; the stdcall return and stack pop are done here. */
int hle_ds_native_call(int id, const uint32_t *a)
{
    static const uint8_t nargs[DS_COUNT] = {
#define HLE_DS(va, name, conv, args) args,
#include "hle_dsound_table.h"
#undef HLE_DS
    };
    uint32_t r = 0;
    if (!hle_ds_native_enabled())
        return 0;
    s_calls[id]++;
    {
        /* RECOMP_DS_LOG=<n>: the first n calls of each function (polling: 20). */
        static int lim = -1;
        static const char *const names[DS_COUNT] = {
#define HLE_DS(va, name, conv, args) #name,
#include "hle_dsound_table.h"
#undef HLE_DS
        };
        if (lim < 0) {
            const char *e = getenv("RECOMP_DS_LOG");
            lim = e ? atoi(e) : 0;
        }
        if (lim && s_calls[id] <= (uint64_t)lim &&
            !((id == DS_IDirectSoundBuffer_GetCurrentPosition || id == DS_IDirectSoundBuffer_GetStatus ||
               id == DS_DirectSoundDoWork) && s_calls[id] > 20))
            fprintf(stderr, "[DS-N] %-36s %08X %08X %08X %08X ret %08X\n", names[id], a[0], a[1], a[2], a[3],
                    MEM32(g_esp));
    }
    if (!ds_do(id, a, &r))
        return 0;
    if (id == DS_DirectSoundDoWork && s_calls[id] % 1800 == 0) {
        int i, playing = 0, used = 0;
        for (i = 0; i < MAX_BUFS; i++) {
            used += s_buf[i].used;
            playing += s_buf[i].used && s_buf[i].playing;
        }
        fprintf(stderr, "[DS-NATIVE] %d buffers, %d playing, %llu mixer frames, %llu Play, %llu GetCurrentPosition\n",
                used, playing, (unsigned long long)s_mix_frames,
                (unsigned long long)s_calls[DS_IDirectSoundBuffer_Play],
                (unsigned long long)s_calls[DS_IDirectSoundBuffer_GetCurrentPosition]);
    }
    g_eax = r;
    g_esp += 4u + 4u * nargs[id];
    return 1;
}

/* The API in C, without the guest stack: 0 = not one of ours. */
static int ds_do(int id, const uint32_t *a, uint32_t *ret)
{
    DsBuf *b;
    uint32_t r = 0;
    EnterCriticalSection(&s_lock);
    switch (id) {
    case DS_DirectSoundCreate:
        if (!s_ds_handle)
            s_ds_handle = alloc_handle(DS_MAGIC, 0);
        if (a[1])
            MEM32(a[1]) = s_ds_handle;
        start_mixer();
        fprintf(stderr, "[DS-NATIVE] DirectSound %08X: library and APU registers bypassed\n", s_ds_handle);
        break;
    case DS_IDirectSound_Release:
        r = 1;
        break;
    case DS_IDirectSound_CreateSoundBuffer:
        r = create_buffer(a[1], a[2]);
        start_mixer();
        break;
    case DS_DirectSoundCreateBuffer:
        r = create_buffer(a[0], a[1]);
        start_mixer();
        break;
    case DS_IDirectSound_SetMixBinHeadroom:
        if (a[1] < NUM_MIXBINS)
            s_headroom[a[1]] = (int)a[2];
        break;
    case DS_IDirectSound_DownloadEffectsImage:
        /* A DSEFFECTIMAGEDESC the title may read: 16 effects whose code,
         * state, Y-memory and scratch segments all point at one zeroed block. */
        if (a[4]) {
            static uint32_t desc;
            if (!desc) {
                uint32_t seg = xbox_HeapAlloc(0x4000, 16), k;
                desc = xbox_HeapAlloc(8 + 16 * 32, 16);
                if (desc && seg) {
                    memset((void *)XBOX_PTR(seg), 0, 0x4000);
                    MEM32(desc) = 16;
                    MEM32(desc + 4) = 0;
                    for (k = 0; k < 16; k++) {
                        uint32_t m = desc + 8 + 32 * k;
                        MEM32(m) = seg; MEM32(m + 4) = 0x400;
                        MEM32(m + 8) = seg; MEM32(m + 12) = 0x400;
                        MEM32(m + 16) = seg; MEM32(m + 20) = 0x400;
                        MEM32(m + 24) = seg; MEM32(m + 28) = 0x400;
                    }
                }
            }
            MEM32(a[4]) = desc;
        }
        break;
    case DS_IDirectSound_SetEffectData:
    case DS_IDirectSound_SetI3DL2Listener:
    case DS_DirectSoundDoWork:
        break;
    case DS_IDirectSoundBuffer_Release:
        if ((b = buf_of(a[0])) != NULL) {
            r = (uint32_t)--b->refs;
            if (!b->refs) {
                MEM32(b->handle) = 0;
                b->used = 0;
            }
        }
        break;
    case DS_IDirectSoundBuffer_SetBufferData:
        if ((b = buf_of(a[0])) != NULL) {
            b->data = a[1];
            b->bytes = a[2];
            b->cached_block = -1;
            if (b->pos >= (double)frames_of_bytes(b, b->bytes))
                b->pos = 0.0;
        }
        break;
    case DS_IDirectSoundBuffer_SetLoopRegion:
        if ((b = buf_of(a[0])) != NULL) {
            b->loop_start = a[1];
            b->loop_len = a[2];
        }
        break;
    case DS_IDirectSoundBuffer_SetCurrentPosition:
        if ((b = buf_of(a[0])) != NULL)
            b->pos = (double)frames_of_bytes(b, a[1]);
        break;
    case DS_IDirectSoundBuffer_GetCurrentPosition:
        if ((b = buf_of(a[0])) != NULL) {
            uint32_t play, write;
            cursors(b, &play, &write);
            if (a[1]) MEM32(a[1]) = play;
            if (a[2]) MEM32(a[2]) = write;
        }
        break;
    case DS_IDirectSoundBuffer_GetStatus:
        if (a[1]) {
            b = buf_of(a[0]);
            MEM32(a[1]) = !b || !b->playing ? 0u : b->paused ? 2u : (1u | (b->looping ? 4u : 0u));
        }
        break;
    case DS_IDirectSoundBuffer_Play:
        if ((b = buf_of(a[0])) != NULL) {
            if (a[3] & 2u)
                b->pos = 0.0;                                  /* DSBPLAY_FROMSTART */
            b->looping = (a[3] & 1u) != 0;
            b->paused = 0;
            if (!b->playing)
                memset(b->svf, 0, sizeof b->svf);
            b->playing = 1;
        }
        break;
    case DS_IDirectSoundBuffer_StopEx:
        /* a[1..2]: REFERENCE_TIME (scheduled stops: none observed), a[3]: flags.
         * No amplitude envelope is ever set, so ENVELOPE releases at once
         * (voice processor: release rate 0 -> voice off); RELEASEWAVEFORM
         * alone leaves the loop and plays out. */
        if ((b = buf_of(a[0])) != NULL) {
            if ((a[3] & 3u) == 2u)
                b->looping = 0;
            else
                b->playing = 0;
        }
        break;
    case DS_IDirectSoundBuffer_Pause:
        if ((b = buf_of(a[0])) != NULL)
            b->paused = a[1] == 1;
        break;
    case DS_IDirectSoundBuffer_SetFrequency:
        if ((b = buf_of(a[0])) != NULL)
            b->pitch = pitch_of(a[1] ? a[1] : (uint32_t)b->rate);
        break;
    case DS_IDirectSoundBuffer_SetMixBins:
        if ((b = buf_of(a[0])) != NULL)
            set_mixbins(b, a[1]);
        break;
    case DS_IDirectSoundBuffer_SetMixBinVolumes_8:
        if ((b = buf_of(a[0])) != NULL && a[1]) {
            uint32_t cnt = MEM32(a[1]), p = MEM32(a[1] + 4), k;
            for (k = 0; k < cnt && p; k++)
                b->binvol[MEM32(p + 8 * k) & (NUM_MIXBINS - 1)] = (int32_t)MEM32(p + 8 * k + 4);
        }
        break;
    case DS_IDirectSoundBuffer_SetEG:
        if ((b = buf_of(a[0])) != NULL && a[1] &&
            (MEM32(a[1]) == 0 || MEM32(a[1] + 32) || MEM32(a[1] + 36))) {
            static int warned;
            if (!warned++)
                fprintf(stderr, "[DS-NATIVE] envelope with effect (EG %u, pitch %d, cutoff %d) not modelled\n",
                        MEM32(a[1]), (int32_t)MEM32(a[1] + 32), (int32_t)MEM32(a[1] + 36));
        }
        break;
    case DS_IDirectSoundBuffer_SetFilter:
        if ((b = buf_of(a[0])) != NULL && a[1]) {
            b->fmode = (int)MEM32(a[1]);
            b->fc0 = (int16_t)MEM32(a[1] + 8);
            b->fc1 = (uint16_t)MEM32(a[1] + 12);
        }
        break;
    default:
        LeaveCriticalSection(&s_lock);
        return 0;
    }
    LeaveCriticalSection(&s_lock);
    *ret = r;
    return 1;
}

/* ------------------------------------------------------- shadow mode */

/* RECOMP_DS_SHADOW=1: the original library keeps playing through the voice
 * processor; every call it receives is replayed here on native buffers
 * mapped to its own, and the APU frame thread runs this mixer beside the
 * voice processor on the same 32-sample frames, comparing the speaker
 * mixbins and the play cursors. Nothing is written to guest memory. */
extern void (*g_apu_shadow_voices)(float mixbins[NUM_MIXBINS][FRAME]);
static uint32_t s_map_lib[MAX_BUFS], s_map_nat[MAX_BUFS];
static uint64_t s_pos_checks, s_pos_diffs;

static uint32_t shadow_handle(uint32_t lib)
{
    int i;
    for (i = 0; i < MAX_BUFS; i++)
        if (s_map_lib[i] == lib && s_map_nat[i])
            return s_map_nat[i];
    return 0;
}

void hle_ds_shadow_after(int id, const uint32_t *a, uint32_t ret)
{
    uint32_t c[6], r, lib, i;
    DsBuf *b;
    if (s_shadow < 0)
        hle_ds_native_enabled();
    if (s_shadow <= 0 || ret)
        return;
    memcpy(c, a, sizeof c);
    switch (id) {
    case DS_DirectSoundCreate:
        if (!g_apu_shadow_voices) {
            g_apu_shadow_voices = mix_voices;
            fprintf(stderr, "[DS-SHADOW] native mixer runs beside the voice processor\n");
        }
        return;
    case DS_DirectSoundCreateBuffer:
    case DS_IDirectSound_CreateSoundBuffer: {
        uint32_t desc = id == DS_DirectSoundCreateBuffer ? a[0] : a[1];
        uint32_t out = id == DS_DirectSoundCreateBuffer ? a[1] : a[2];
        lib = out ? MEM32(out) : 0;
        if (!lib)
            return;
        EnterCriticalSection(&s_lock);
        create_buffer(desc, 0);
        for (i = 0; i < MAX_BUFS; i++)
            if (s_buf[i].used && !s_map_nat[i] && s_buf[i].handle) {
                s_map_lib[i] = lib;
                s_map_nat[i] = s_buf[i].handle;
                break;
            }
        LeaveCriticalSection(&s_lock);
        return;
    }
    case DS_IDirectSoundBuffer_GetCurrentPosition: {
        /* The library's answer is in guest memory now; compare ours. */
        uint32_t nat = shadow_handle(a[0]);
        if (!nat || !(b = buf_of(nat)) || (!a[1] && !a[2]))
            return;
        EnterCriticalSection(&s_lock);
        {
            uint32_t play, write, lib = a[2] ? MEM32(a[2]) : MEM32(a[1]), mine;
            int32_t d;
            cursors(b, &play, &write);
            mine = a[2] ? write : play;
            d = (int32_t)(mine - lib);
            if (b->bytes && d > (int32_t)b->bytes / 2) d -= (int32_t)b->bytes;     /* across the wrap */
            if (b->bytes && d < -(int32_t)b->bytes / 2) d += (int32_t)b->bytes;
            s_pos_checks++;
            if (d > 1536 || d < -1536) {
                if (s_pos_diffs++ < 20)
                    fprintf(stderr, "[DS-SHADOW] %s cursor native %u library %u (buffer %08X, %u bytes, %s)\n",
                            a[2] ? "write" : "play", mine, lib, a[0], b->bytes, b->playing ? "playing" : "stopped");
            }
        }
        LeaveCriticalSection(&s_lock);
        return;
    }
    case DS_IDirectSoundBuffer_GetStatus:
    case DS_IDirectSound_DownloadEffectsImage:
        return;
    case DS_IDirectSoundBuffer_Release:
        if ((c[0] = shadow_handle(a[0])) != 0) {
            for (i = 0; i < MAX_BUFS; i++)
                if (s_map_lib[i] == a[0])
                    s_map_lib[i] = s_map_nat[i] = 0;
            ds_do(id, c, &r);
        }
        return;
    default:
        if (id == DS_IDirectSound_SetMixBinHeadroom || id == DS_IDirectSound_SetEffectData ||
            id == DS_IDirectSound_SetI3DL2Listener || id == DS_DirectSoundDoWork || id == DS_IDirectSound_Release) {
            ds_do(id, c, &r);
            return;
        }
        if (!(c[0] = shadow_handle(a[0])))
            return;
        ds_do(id, c, &r);
        return;
    }
}

/* Called by the frame thread with both mixes of one frame. */
void hle_ds_shadow_compare(float lib[NUM_MIXBINS][FRAME], float nat[NUM_MIXBINS][FRAME])
{
    static double xy, xx, yy, ex;
    static uint64_t frames;
    int bin, i;
    for (bin = 0; bin < 6; bin++)
        for (i = 0; i < FRAME; i++) {
            double x = lib[bin][i], y = nat[bin][i];
            xy += x * y; xx += x * x; yy += y * y; ex += (y - x) * (y - x);
        }
    if (++frames % 7500 == 0) {                /* ~5 s */
        double corr = xx > 0 && yy > 0 ? xy / sqrt(xx * yy) : 0.0, gain = xx > 0 ? xy / xx : 0.0;
        double snr = ex > 0 && xx > 0 ? 10.0 * log10(xx / ex) : 999.0;
        fprintf(stderr, "[DS-SHADOW] frames %llu: corr %.5f gain %.4f SNR %.1f dB rms lib %.4f native %.4f, "
                "cursor checks %llu off %llu\n", (unsigned long long)frames, corr, gain, snr,
                sqrt(xx / (7500.0 * 6 * FRAME)), sqrt(yy / (7500.0 * 6 * FRAME)),
                (unsigned long long)s_pos_checks, (unsigned long long)s_pos_diffs);
        xy = xx = yy = ex = 0.0;
    }
}
