/*
 * QEMU MCPX Audio Processing Unit implementation
 *
 * Copyright (c) 2012 espes
 * Copyright (c) 2018-2019 Jannik Vogel
 * Copyright (c) 2019-2025 Matt Borgerson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "apu_state.h"
#include "fpconv.h"
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include "dsp/debug.h"

static const int16_t ep_silence[256][2] = { 0 };

/* RECOMP_WRITE_TRACE=1: each guest page the DSP writes is logged once, with
 * time, so a corrupted heap address can be matched against DMA targets. */
static void trace_dsp_write_page(hwaddr paddr, size_t bytes, uint32_t addr)
{
    static int on = -1;
    static uint32_t seen[1024];
    static unsigned n;
    if (on < 0) on = getenv("RECOMP_WRITE_TRACE") != NULL;
    if (!on) return;
    uint32_t page = (uint32_t)(paddr >> 12);
    for (unsigned i = 0; i < n; i++)
        if (seen[i] == page) return;
    if (n < 1024) seen[n++] = page;
    fprintf(stderr, "[WTRACE] dsp-dma t=%llu phys=%08X bytes=%zu scratch=%08X\n",
            (unsigned long long)clock(), (unsigned)paddr, bytes, addr);
}

/* RECOMP_GP_OBSERVE=1: what the guest sees of the GP -- CPU reads of its
 * memories and registers (count, value changes, last value) and what its DMA
 * writes to RAM (scratch pages, output FIFOs). A summary every 3000 frames.
 * To learn which GP side effects the title depends on, so the GP program can
 * be replaced by native code that produces just those. */
static int s_gpo = -1;
typedef struct { uint32_t n, changes, last; } GpoCell;
static GpoCell s_gpo_x[0x1000], s_gpo_y[0x800], s_gpo_p[0x1000], s_gpo_mix[0x400], s_gpo_r[0x4000];
static uint32_t s_gpo_sw_bytes[1024], s_gpo_sr_bytes[1024];   /* scratch per 4 KB page (first 4 MB) */
static uint32_t s_gpo_sw_other, s_gpo_sr_other;
static uint64_t s_gpo_fifo_out[4], s_gpo_fifo_in[4];
static uint32_t s_gpo_frames, s_gpo_gp_ran;
static uint32_t s_gpo_sw_first[1024];                      /* first dword written to the page */
/* Per scratch dword (first 256 KB): last value written, writes, changes, and
 * small positive steps (1..4096) -- a counter the title could pace on. */
#define GPO_DW (64 * 1024)
static uint32_t s_gpo_val[GPO_DW], s_gpo_wr[GPO_DW], s_gpo_ch[GPO_DW], s_gpo_inc[GPO_DW];
static uint64_t s_gpo_incsum[GPO_DW];

static int gpo_on(void)
{
    if (s_gpo < 0) s_gpo = getenv("RECOMP_GP_OBSERVE") != NULL;
    return s_gpo;
}

static void gpo_cell(GpoCell *c, uint32_t v)
{
    if (c->n && c->last != v) c->changes++;
    c->n++;
    c->last = v;
}

static void gpo_top(const char *name, GpoCell *t, unsigned count, unsigned mult)
{
    unsigned shown = 0, i;
    char used[0x4000];
    memset(used, 0, sizeof used);
    fprintf(stderr, "[GP-OBS] %s reads:", name);
    while (shown < 12) {
        unsigned best = ~0u;
        for (i = 0; i < count; i++)
            if (!used[i] && t[i].n && (best == ~0u || t[i].n > t[best].n)) best = i;
        if (best == ~0u) break;
        used[best] = 1;
        fprintf(stderr, " %X:n%u/ch%u=%06X", best * mult, t[best].n, t[best].changes, t[best].last);
        shown++;
    }
    fprintf(stderr, "\n");
}

/* Which guest pages each VP voice reads samples from (apu_vp.c hook). */
static uint32_t s_gpo_vp_page[256], s_gpo_vp_seen[256];
void mcpx_gpo_vp_read(unsigned v, uint64_t addr)
{
    if (!gpo_on() || v >= 256) return;
    s_gpo_vp_page[v] = (uint32_t)(addr & 0x0FFFFFFFu) >> 12;
    s_gpo_vp_seen[v] = s_gpo_frames + 1;
}

static void gpo_frame(MCPXAPUState *d)
{
    unsigned i;
    if (!gpo_on() || ++s_gpo_frames % 3000) return;
    fprintf(stderr, "[GP-OBS] frame %u (GP ran %u of the last 3000)\n", s_gpo_frames, s_gpo_gp_ran);
    s_gpo_gp_ran = 0;
    { extern void mcpx_gpw_report(void); mcpx_gpw_report(); }
    {   /* GP scratch pages (SGE) vs pages VP voices read */
        static uint32_t gp_page[1024];
        unsigned v, pg, npg = d->regs[NV_PAPU_GPSMAXSGE] + 1;
        if (npg > 1024) npg = 1024;
        for (pg = 0; pg < npg; pg++)
            gp_page[pg] = (ldl_le_phys(address_space_memory, d->regs[NV_PAPU_GPSADDR] + pg * 8) & 0x0FFFFFFFu) >> 12;
        fprintf(stderr, "[GP-OBS] scratch phys p0=%05X p8=%05X p9=%05X p10=%05X p11=%05X p12=%05X\n",
                gp_page[0], gp_page[8], gp_page[9], gp_page[10], gp_page[11], gp_page[12]);
        fprintf(stderr, "[GP-OBS] voices on GP scratch:");
        for (v = 0; v < 256; v++) {
            if (!s_gpo_vp_seen[v] || s_gpo_frames - (s_gpo_vp_seen[v] - 1) > 3000) continue;
            for (pg = 0; pg < npg; pg++)
                if (gp_page[pg] == s_gpo_vp_page[v]) {
                    fprintf(stderr, " v%u<-p%u(w%u r%u)", v, pg, s_gpo_sw_bytes[pg], s_gpo_sr_bytes[pg]);
                    break;
                }
        }
        fprintf(stderr, "\n[GP-OBS] active voice pages:");
        for (v = 0; v < 256; v++)
            if (s_gpo_vp_seen[v] && s_gpo_frames - (s_gpo_vp_seen[v] - 1) <= 3000)
                fprintf(stderr, " v%u:%05X", v, s_gpo_vp_page[v]);
        fprintf(stderr, "\n");
    }
    {   /* counters: >= 90% of changes are small positive steps, >= 100 changes */
        unsigned shown = 0, dw;
        fprintf(stderr, "[GP-OBS] counters:");
        for (dw = 0; dw < GPO_DW && shown < 24; dw++)
            if (s_gpo_ch[dw] >= 100 && s_gpo_inc[dw] * 10 >= s_gpo_ch[dw] * 9) {
                fprintf(stderr, " %X:wr%u/ch%u/step%.1f=%08X", dw * 4, s_gpo_wr[dw], s_gpo_ch[dw],
                        (double)s_gpo_incsum[dw] / s_gpo_inc[dw], s_gpo_val[dw]);
                shown++;
            }
        fprintf(stderr, "\n");
        memset(s_gpo_wr, 0, sizeof s_gpo_wr); memset(s_gpo_ch, 0, sizeof s_gpo_ch);
        memset(s_gpo_inc, 0, sizeof s_gpo_inc); memset(s_gpo_incsum, 0, sizeof s_gpo_incsum);
    }
    gpo_top("X", s_gpo_x, 0x1000, 1);
    gpo_top("Y", s_gpo_y, 0x800, 1);
    gpo_top("P", s_gpo_p, 0x1000, 1);
    gpo_top("MIX", s_gpo_mix, 0x400, 1);
    gpo_top("REG", s_gpo_r, 0x4000, 4);
    fprintf(stderr, "[GP-OBS] scratch writes:");
    for (i = 0; i < 1024; i++)
        if (s_gpo_sw_bytes[i]) fprintf(stderr, " p%u:%u(first %08X)", i, s_gpo_sw_bytes[i], s_gpo_sw_first[i]);
    fprintf(stderr, " other:%u\n[GP-OBS] scratch reads:", s_gpo_sw_other);
    for (i = 0; i < 1024; i++)
        if (s_gpo_sr_bytes[i]) fprintf(stderr, " p%u:%u", i, s_gpo_sr_bytes[i]);
    fprintf(stderr, " other:%u\n[GP-OBS] fifo out %llu %llu %llu %llu in %llu %llu\n", s_gpo_sr_other,
            (unsigned long long)s_gpo_fifo_out[0], (unsigned long long)s_gpo_fifo_out[1],
            (unsigned long long)s_gpo_fifo_out[2], (unsigned long long)s_gpo_fifo_out[3],
            (unsigned long long)s_gpo_fifo_in[0], (unsigned long long)s_gpo_fifo_in[1]);
    memset(s_gpo_x, 0, sizeof s_gpo_x); memset(s_gpo_y, 0, sizeof s_gpo_y);
    memset(s_gpo_p, 0, sizeof s_gpo_p); memset(s_gpo_mix, 0, sizeof s_gpo_mix);
    memset(s_gpo_r, 0, sizeof s_gpo_r);
    memset(s_gpo_sw_bytes, 0, sizeof s_gpo_sw_bytes); memset(s_gpo_sr_bytes, 0, sizeof s_gpo_sr_bytes);
    s_gpo_sw_other = s_gpo_sr_other = 0;
    memset(s_gpo_fifo_out, 0, sizeof s_gpo_fifo_out); memset(s_gpo_fifo_in, 0, sizeof s_gpo_fifo_in);
}

static void scatter_gather_rw(MCPXAPUState *d, hwaddr sge_base,
                              unsigned int max_sge, uint8_t *ptr, uint32_t addr,
                              size_t len, bool dir)
{
    unsigned int page_entry = addr / TARGET_PAGE_SIZE;
    unsigned int offset_in_page = addr % TARGET_PAGE_SIZE;
    unsigned int bytes_to_copy = TARGET_PAGE_SIZE - offset_in_page;

    while (len > 0) {
        assert(page_entry <= max_sge);

        uint32_t prd_address = ldl_le_phys(address_space_memory,
                                           sge_base + page_entry * 8 + 0);
        // uint32_t prd_control = ldl_le_phys(&address_space_memory,
        //                                     sge_base + page_entry * 8 + 4);

        hwaddr paddr = prd_address + offset_in_page;

        if (bytes_to_copy > len) {
            bytes_to_copy = len;
        }

        assert((paddr & 0x0FFFFFFFu) + bytes_to_copy < 0x04000000u);

        if (dir) {
            trace_dsp_write_page(paddr, bytes_to_copy, addr);
            memcpy(mcpx_apu_phys(paddr), ptr, bytes_to_copy);
            /* Guest RAM is shared directly; no QEMU dirty-page tracker. */
        } else {
            memcpy(ptr, mcpx_apu_phys(paddr), bytes_to_copy);
        }

        ptr += bytes_to_copy;
        len -= bytes_to_copy;

        /* After the first iteration, we are page aligned */
        page_entry += 1;
        bytes_to_copy = TARGET_PAGE_SIZE;
        offset_in_page = 0;
    }
}

static void gp_scratch_rw(void *opaque, uint8_t *ptr, uint32_t addr, size_t len,
                          bool dir)
{
    MCPXAPUState *d = opaque;
    // fprintf(stderr, "GP %s scratch 0x%x bytes (0x%x words) at %x (0x%x words)\n", dir ? "writing to" : "reading from", len, len/4, addr, addr/4);
    if (gpo_on() && (addr >> 12) == 0) {
        /* Page 0 is DirectSound's mailbox (command at +0x800, reply at +0x804):
         * log the GP's side of every exchange, with the command beside it. */
        static unsigned shown;
        static uint32_t last_cmd = 0xFFFFFFFFu;
        uint32_t w0 = 0, cmd = 0, pa = ldl_le_phys(address_space_memory, d->regs[NV_PAPU_GPSADDR]);
        if (len >= 4) memcpy(&w0, ptr, 4);
        if (pa) memcpy(&cmd, mcpx_apu_phys(pa + 0x800), 4);
        if ((dir || cmd != last_cmd) && shown++ < 200) {
            last_cmd = cmd;
            fprintf(stderr, "[GP-MBOX] frame %u GP %s p0+0x%03X len %u first %08X | mailbox cmd %08X\n",
                    s_gpo_frames, dir ? "writes" : "reads", addr, (unsigned)len, w0, cmd);
        }
    }
    if (gpo_on()) {
        unsigned pg = addr >> 12;
        if (dir) {
            if (pg < 1024) {
                if (!s_gpo_sw_bytes[pg] && len >= 4) memcpy(&s_gpo_sw_first[pg], ptr, 4);
                s_gpo_sw_bytes[pg] += (uint32_t)len;
            } else s_gpo_sw_other += (uint32_t)len;
            for (size_t k = 0; k + 4 <= len; k += 4) {
                uint32_t dw = (addr + (uint32_t)k) / 4, v;
                if (dw >= GPO_DW) break;
                memcpy(&v, ptr + k, 4);
                if (s_gpo_wr[dw]++ && v != s_gpo_val[dw]) {
                    uint32_t step = v - s_gpo_val[dw];
                    s_gpo_ch[dw]++;
                    if (step >= 1 && step <= 4096) { s_gpo_inc[dw]++; s_gpo_incsum[dw] += step; }
                }
                s_gpo_val[dw] = v;
            }
        } else {
            if (pg < 1024) s_gpo_sr_bytes[pg] += (uint32_t)len; else s_gpo_sr_other += (uint32_t)len;
        }
    }
    scatter_gather_rw(d, d->regs[NV_PAPU_GPSADDR], d->regs[NV_PAPU_GPSMAXSGE],
                      ptr, addr, len, dir);
}

static void ep_scratch_rw(void *opaque, uint8_t *ptr, uint32_t addr, size_t len,
                          bool dir)
{
    MCPXAPUState *d = opaque;
    // fprintf(stderr, "EP %s scratch 0x%x bytes (0x%x words) at %x (0x%x words)\n", dir ? "writing to" : "reading from", len, len/4, addr, addr/4);
    scatter_gather_rw(d, d->regs[NV_PAPU_EPSADDR], d->regs[NV_PAPU_EPSMAXSGE],
                      ptr, addr, len, dir);
}

static uint32_t circular_scatter_gather_rw(MCPXAPUState *d, hwaddr sge_base,
                                           unsigned int max_sge, uint8_t *ptr,
                                           uint32_t base, uint32_t end,
                                           uint32_t cur, size_t len, bool dir)
{
    while (len > 0) {
        unsigned int bytes_to_copy = end - cur;

        if (bytes_to_copy > len) {
            bytes_to_copy = len;
        }

        DPRINTF("circular scatter gather %s in range 0x%x - 0x%x at 0x%x of "
                "length 0x%x / 0x%lx bytes\n",
                dir ? "write" : "read", base, end, cur, bytes_to_copy, len);

        assert((cur >= base) && ((cur + bytes_to_copy) <= end));
        scatter_gather_rw(d, sge_base, max_sge, ptr, cur, bytes_to_copy, dir);

        ptr += bytes_to_copy;
        len -= bytes_to_copy;

        /* After the first iteration we might have to wrap */
        cur += bytes_to_copy;
        if (cur >= end) {
            assert(cur == end);
            cur = base;
        }
    }

    return cur;
}

static void gp_fifo_rw(void *opaque, uint8_t *ptr, unsigned int index,
                       size_t len, bool dir)
{
    MCPXAPUState *d = opaque;
    uint32_t base;
    uint32_t end;
    hwaddr cur_reg;
    if (dir) {
        assert(index < GP_OUTPUT_FIFO_COUNT);
        base = GET_MASK(d->regs[NV_PAPU_GPOFBASE0 + 0x10 * index],
                        NV_PAPU_GPOFBASE0_VALUE);
        end = GET_MASK(d->regs[NV_PAPU_GPOFEND0 + 0x10 * index],
                       NV_PAPU_GPOFEND0_VALUE);
        cur_reg = NV_PAPU_GPOFCUR0 + 0x10 * index;
    } else {
        assert(index < GP_INPUT_FIFO_COUNT);
        base = GET_MASK(d->regs[NV_PAPU_GPIFBASE0 + 0x10 * index],
                        NV_PAPU_GPOFBASE0_VALUE);
        end = GET_MASK(d->regs[NV_PAPU_GPIFEND0 + 0x10 * index],
                       NV_PAPU_GPOFEND0_VALUE);
        cur_reg = NV_PAPU_GPIFCUR0 + 0x10 * index;
    }

    uint32_t cur = GET_MASK(d->regs[cur_reg], NV_PAPU_GPOFCUR0_VALUE);
    if (gpo_on()) {
        if (dir) s_gpo_fifo_out[index & 3] += len; else s_gpo_fifo_in[index & 1] += len;
    }

    // fprintf(stderr, "GP %s fifo #%d, base = %x, end = %x, cur = %x, len = %x\n",
    //     dir ? "writing to" : "reading from", index,
    //     base, end, cur, len);

    /* DSP hangs if current >= end; but forces current >= base */
    assert(cur < end);
    if (cur < base) {
        cur = base;
    }

    cur = circular_scatter_gather_rw(d,
        d->regs[NV_PAPU_GPFADDR], d->regs[NV_PAPU_GPFMAXSGE],
        ptr, base, end, cur, len, dir);

    SET_MASK(d->regs[cur_reg], NV_PAPU_GPOFCUR0_VALUE, cur);
}

static bool ep_sink_samples(MCPXAPUState *d, uint8_t *ptr, size_t len)
{
    if (d->monitor.point == MCPX_APU_DEBUG_MON_AC97) {
        return false;
    } else if ((d->monitor.point == MCPX_APU_DEBUG_MON_EP) ||
        (d->monitor.point == MCPX_APU_DEBUG_MON_GP_OR_EP)) {
        assert(len == sizeof(d->monitor.frame_buf));
        memcpy(d->monitor.frame_buf, ptr, len);
    }

    return true;
}

static void ep_fifo_rw(void *opaque, uint8_t *ptr, unsigned int index,
                       size_t len, bool dir)
{
    MCPXAPUState *d = opaque;
    uint32_t base;
    uint32_t end;
    hwaddr cur_reg;
    if (dir) {
        assert(index < EP_OUTPUT_FIFO_COUNT);
        base = GET_MASK(d->regs[NV_PAPU_EPOFBASE0 + 0x10 * index],
                        NV_PAPU_GPOFBASE0_VALUE);
        end = GET_MASK(d->regs[NV_PAPU_EPOFEND0 + 0x10 * index],
                       NV_PAPU_GPOFEND0_VALUE);
        cur_reg = NV_PAPU_EPOFCUR0 + 0x10 * index;
    } else {
        assert(index < EP_INPUT_FIFO_COUNT);
        base = GET_MASK(d->regs[NV_PAPU_EPIFBASE0 + 0x10 * index],
                        NV_PAPU_GPOFBASE0_VALUE);
        end = GET_MASK(d->regs[NV_PAPU_EPIFEND0 + 0x10 * index],
                       NV_PAPU_GPOFEND0_VALUE);
        cur_reg = NV_PAPU_EPIFCUR0 + 0x10 * index;
    }

    uint32_t cur = GET_MASK(d->regs[cur_reg], NV_PAPU_GPOFCUR0_VALUE);

    // fprintf(stderr, "EP %s fifo #%d, base = %x, end = %x, cur = %x, len = %x\n",
    //     dir ? "writing to" : "reading from", index,
    //     base, end, cur, len);

    if (dir && index == 0) {
        bool did_sink = ep_sink_samples(d, ptr, len);
        if (did_sink) {
            /* Since we are sinking, push silence out */
            assert(len <= sizeof(ep_silence));
            ptr = (uint8_t*)ep_silence;
        }
    }

    /* DSP hangs if current >= end; but forces current >= base */
    if (cur >= end) {
        cur = cur % (end - base);
    }
    if (cur < base) {
        cur = base;
    }

    cur = circular_scatter_gather_rw(d,
        d->regs[NV_PAPU_EPFADDR], d->regs[NV_PAPU_EPFMAXSGE],
        ptr, base, end, cur, len, dir);

    SET_MASK(d->regs[cur_reg], NV_PAPU_GPOFCUR0_VALUE, cur);
}

static void proc_rst_write(DSPState *dsp, uint32_t oldval, uint32_t val)
{
    if (!(val & NV_PAPU_GPRST_GPRST) || !(val & NV_PAPU_GPRST_GPDSPRST)) {
        dsp_reset(dsp);
    } else if (
        (!(oldval & NV_PAPU_GPRST_GPRST) || !(oldval & NV_PAPU_GPRST_GPDSPRST))
        && ((val & NV_PAPU_GPRST_GPRST) && (val & NV_PAPU_GPRST_GPDSPRST))) {
        dsp_bootstrap(dsp);
    }
}

/* Frame number of the last guest access to the GP aperture (native audio
 * runs the GP only around DirectSound's conversations with it). */
static volatile uint32_t s_gp_frame, s_gp_last_access;

/* Global Processor - programmable DSP */
static uint64_t gp_read(void *opaque, hwaddr addr, unsigned int size)
{
    MCPXAPUState *d = opaque;
    s_gp_last_access = s_gp_frame;

    assert(size == 4);
    assert(addr % 4 == 0);

    uint64_t r = 0;
    switch (addr) {
    case NV_PAPU_GPXMEM ... NV_PAPU_GPXMEM + 0x1000 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_GPXMEM) / 4;
        r = dsp_read_memory(d->gp.dsp, 'X', xaddr);
        // fprintf(stderr, "read GP NV_PAPU_GPXMEM [%x] -> %x\n", xaddr, r);
        break;
    }
    case NV_PAPU_GPMIXBUF ... NV_PAPU_GPMIXBUF + 0x400 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_GPMIXBUF) / 4;
        r = dsp_read_memory(d->gp.dsp, 'X', GP_DSP_MIXBUF_BASE + xaddr);
        // fprintf(stderr, "read GP NV_PAPU_GPMIXBUF [%x] -> %x\n", xaddr, r);
        break;
    }
    case NV_PAPU_GPYMEM ... NV_PAPU_GPYMEM + 0x800 * 4 - 1: {
        uint32_t yaddr = (addr - NV_PAPU_GPYMEM) / 4;
        r = dsp_read_memory(d->gp.dsp, 'Y', yaddr);
        // fprintf(stderr, "read GP NV_PAPU_GPYMEM [%x] -> %x\n", yaddr, r);
        break;
    }
    case NV_PAPU_GPPMEM ... NV_PAPU_GPPMEM + 0x1000 * 4 - 1: {
        uint32_t paddr = (addr - NV_PAPU_GPPMEM) / 4;
        r = dsp_read_memory(d->gp.dsp, 'P', paddr);
        // fprintf(stderr, "read GP NV_PAPU_GPPMEM [%x] -> %x\n", paddr, r);
        break;
    }
    default:
        r = d->gp.regs[addr];
        break;
    }
    if (gpo_on()) {
        if (addr < NV_PAPU_GPMIXBUF) gpo_cell(&s_gpo_x[(addr / 4) & 0xFFF], (uint32_t)r);
        else if (addr < NV_PAPU_GPYMEM) gpo_cell(&s_gpo_mix[((addr - NV_PAPU_GPMIXBUF) / 4) & 0x3FF], (uint32_t)r);
        else if (addr < NV_PAPU_GPPMEM) gpo_cell(&s_gpo_y[((addr - NV_PAPU_GPYMEM) / 4) & 0x7FF], (uint32_t)r);
        else if (addr < NV_PAPU_GPPMEM + 0x4000) gpo_cell(&s_gpo_p[((addr - NV_PAPU_GPPMEM) / 4) & 0xFFF], (uint32_t)r);
        else gpo_cell(&s_gpo_r[(addr / 4) & 0x3FFF], (uint32_t)r);
    }
    DPRINTF("mcpx apu GP: read [0x%" HWADDR_PRIx "] -> 0x%lx\n", addr, r);

    return r;
}

static void gp_write(void *opaque, hwaddr addr, uint64_t val, unsigned int size)
{
    MCPXAPUState *d = opaque;
    s_gp_last_access = s_gp_frame;

    qemu_mutex_lock(&d->lock);

    assert(size == 4);
    assert(addr % 4 == 0);

    DPRINTF("mcpx apu GP: [0x%" HWADDR_PRIx "] = 0x%lx\n", addr, val);

    switch (addr) {
    case NV_PAPU_GPXMEM ... NV_PAPU_GPXMEM + 0x1000 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_GPXMEM) / 4;
        // fprintf(stderr, "gp write xmem %x = %x\n", xaddr, val);
        dsp_write_memory(d->gp.dsp, 'X', xaddr, val);
        break;
    }
    case NV_PAPU_GPMIXBUF ... NV_PAPU_GPMIXBUF + 0x400 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_GPMIXBUF) / 4;
        // fprintf(stderr, "gp write xmixbuf %x = %x\n", xaddr, val);
        dsp_write_memory(d->gp.dsp, 'X', GP_DSP_MIXBUF_BASE + xaddr, val);
        break;
    }
    case NV_PAPU_GPYMEM ... NV_PAPU_GPYMEM + 0x800 * 4 - 1: {
        uint32_t yaddr = (addr - NV_PAPU_GPYMEM) / 4;
        // fprintf(stderr, "gp write ymem %x = %x\n", yaddr, val);
        dsp_write_memory(d->gp.dsp, 'Y', yaddr, val);
        break;
    }
    case NV_PAPU_GPPMEM ... NV_PAPU_GPPMEM + 0x1000 * 4 - 1: {
        uint32_t paddr = (addr - NV_PAPU_GPPMEM) / 4;
        // fprintf(stderr, "gp write pmem %x = %x\n", paddr, val);
        dsp_write_memory(d->gp.dsp, 'P', paddr, val);
        break;
    }
    case NV_PAPU_GPRST:
        proc_rst_write(d->gp.dsp, d->gp.regs[NV_PAPU_GPRST], val);
        d->gp.regs[NV_PAPU_GPRST] = val;
        break;
    default:
        d->gp.regs[addr] = val;
        break;
    }

    qemu_mutex_unlock(&d->lock);
}



/* Encode Processor - encoding DSP */
static uint64_t ep_read(void *opaque, hwaddr addr, unsigned int size)
{
    MCPXAPUState *d = opaque;

    assert(size == 4);
    assert(addr % 4 == 0);

    uint64_t r = 0;
    switch (addr) {
    case NV_PAPU_EPXMEM ... NV_PAPU_EPXMEM + 0xC00 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_EPXMEM) / 4;
        r = dsp_read_memory(d->ep.dsp, 'X', xaddr);
        // fprintf(stderr, "read EP  NV_PAPU_EPXMEM [%x] -> %x\n", xaddr, r);
        break;
    }
    case NV_PAPU_EPYMEM ... NV_PAPU_EPYMEM + 0x100 * 4 - 1: {
        uint32_t yaddr = (addr - NV_PAPU_EPYMEM) / 4;
        r = dsp_read_memory(d->ep.dsp, 'Y', yaddr);
        // fprintf(stderr, "read EP  NV_PAPU_EPYMEM [%x] -> %x\n", yaddr, r);
        break;
    }
    case NV_PAPU_EPPMEM ... NV_PAPU_EPPMEM + 0x1000 * 4 - 1: {
        uint32_t paddr = (addr - NV_PAPU_EPPMEM) / 4;
        r = dsp_read_memory(d->ep.dsp, 'P', paddr);
        // fprintf(stderr, "read EP  NV_PAPU_EPPMEM [%x] -> %x\n", paddr, r);
        break;
    }
    default:
        r = d->ep.regs[addr];
        break;
    }
    DPRINTF("mcpx apu EP: read [0x%" HWADDR_PRIx "] -> 0x%lx\n", addr, r);

    return r;
}

static void ep_write(void *opaque, hwaddr addr, uint64_t val, unsigned int size)
{
    MCPXAPUState *d = opaque;

    qemu_mutex_lock(&d->lock);

    assert(size == 4);
    assert(addr % 4 == 0);

    DPRINTF("mcpx apu EP: [0x%" HWADDR_PRIx "] = 0x%lx\n", addr, val);

    switch (addr) {
    case NV_PAPU_EPXMEM ... NV_PAPU_EPXMEM + 0xC00 * 4 - 1: {
        uint32_t xaddr = (addr - NV_PAPU_EPXMEM) / 4;
        dsp_write_memory(d->ep.dsp, 'X', xaddr, val);
        // fprintf(stderr, "ep write xmem %x = %x\n", xaddr, val);
        break;
    }
    case NV_PAPU_EPYMEM ... NV_PAPU_EPYMEM + 0x100 * 4 - 1: {
        uint32_t yaddr = (addr - NV_PAPU_EPYMEM) / 4;
        dsp_write_memory(d->ep.dsp, 'Y', yaddr, val);
        // fprintf(stderr, "ep write ymem %x = %x\n", yaddr, val);
        break;
    }
    case NV_PAPU_EPPMEM ... NV_PAPU_EPPMEM + 0x1000 * 4 - 1: {
        uint32_t paddr = (addr - NV_PAPU_EPPMEM) / 4;
        // fprintf(stderr, "ep write pmem %x = %x\n", paddr, val);
        dsp_write_memory(d->ep.dsp, 'P', paddr, val);
        break;
    }
    case NV_PAPU_EPRST:
        proc_rst_write(d->ep.dsp, d->ep.regs[NV_PAPU_EPRST], val);
        d->ep.regs[NV_PAPU_EPRST] = val;
        d->ep_frame_div = 0; /* FIXME: Still unsure about frame sync */
        break;
    default:
        d->ep.regs[addr] = val;
        break;
    }

    qemu_mutex_unlock(&d->lock);
}



/* Native final mix: the six speaker mixbins (FL, FR, C, LFE, BL, BR) folded
 * to stereo with the ITU coefficients, LFE dropped. FIFA Street's EA engine
 * mixes in software into a 6-channel buffer played on three stereo voices
 * routed to exactly these bins, so this is the whole program the GP/EP DSPs
 * ran for it, as C. g_apu_native_buf holds one EP frame (8 x 32 samples). */
int16_t g_apu_native_buf[NUM_SAMPLES_PER_FRAME * 8][2];

/* The fold sums up to 2.4x full scale when every speaker channel is loud
 * (crowd + commentary + music in a match); clamping that is audible
 * clipping. A peak limiter -- instant attack, ~60 ms release -- keeps the
 * sum under full scale and leaves anything quieter untouched. */
static float s_limit_gain = 1.0f;
static unsigned long long s_limit_hits, s_limit_samples;

void mcpx_apu_native_mix(MCPXAPUState *d, float mixbins[NUM_MIXBINS][NUM_SAMPLES_PER_FRAME])
{
    int off = (d->ep_frame_div % 8) * NUM_SAMPLES_PER_FRAME;
    for (int i = 0; i < NUM_SAMPLES_PER_FRAME; i++) {
        float c = 0.70710678f * mixbins[2][i];
        float l = mixbins[0][i] + c + 0.70710678f * mixbins[4][i];
        float r = mixbins[1][i] + c + 0.70710678f * mixbins[5][i];
        float peak = fabsf(l) > fabsf(r) ? fabsf(l) : fabsf(r);
        if (peak * s_limit_gain > 0.98f) {
            s_limit_gain = 0.98f / peak;
            s_limit_hits++;
        } else {
            s_limit_gain += (1.0f - s_limit_gain) * 0.00035f;
        }
        l *= s_limit_gain;
        r *= s_limit_gain;
        if (++s_limit_samples % (48000ull * 30) == 0) {
            fprintf(stderr, "[APU-LIMIT] %llu of the last 30 s of samples needed gain reduction, gain now %.3f\n",
                    s_limit_hits, s_limit_gain);
            s_limit_hits = 0;
        }
        l = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
        r = r > 1.0f ? 1.0f : r < -1.0f ? -1.0f : r;
        g_apu_native_buf[off + i][0] = (int16_t)(l * 32767.0f);
        g_apu_native_buf[off + i][1] = (int16_t)(r * 32767.0f);
    }
}

/* RECOMP_AUDIO_NATIVE=1: the native mix is what reaches the speakers and the
 * GP/EP programs are not executed. The DSP state keeps receiving DirectSound's
 * MMIO (program downloads, resets, FIFO setup) so the library sees the same
 * registers; only the 56300 interpreters stop running. Measured against the
 * interpreted pipeline on the same frames: correlation 1.0, gain 1.0002,
 * 72 dB SNR, 480 samples earlier (the DSP pipeline's latency). */
int mcpx_apu_audio_native(void)
{
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("RECOMP_AUDIO_NATIVE");
        on = e && *e && *e != '0';
    }
    return on;
}

/* The GP's part that a title can observe, natively (RECOMP_AUDIO_NATIVE).
 *
 * Measured on FIFA Street (RECOMP_GP_OBSERVE / RECOMP_GP_WATCH): the CPU never
 * reads the GP aperture, no VP voice plays from GP scratch, and guest code
 * touches the GP's scratch only while DirectSound starts the effects image --
 * it writes its mailbox at scratch +0x800 and waits on +0x804. The GP program's
 * whole visible reply is to clear those 24 bytes (and then run effects whose
 * output only feeds the EP, which the native mix replaces). So: while the GP
 * is out of reset, clear a non-empty mailbox; the 56300 never executes.
 * RECOMP_AUDIO_GP=1 runs the interpreted GP as before. */
static void native_gp_mailbox(MCPXAPUState *d)
{
    uint8_t box[24];
    static const uint8_t zero[24];
    if (!(d->gp.regs[NV_PAPU_GPRST] & NV_PAPU_GPRST_GPRST) ||
        !(d->gp.regs[NV_PAPU_GPRST] & NV_PAPU_GPRST_GPDSPRST) ||
        !d->regs[NV_PAPU_GPSADDR] ||
        !ldl_le_phys(address_space_memory, d->regs[NV_PAPU_GPSADDR]))
        return;
    scatter_gather_rw(d, d->regs[NV_PAPU_GPSADDR], d->regs[NV_PAPU_GPSMAXSGE], box, 0x800, sizeof box, false);
    if (memcmp(box, zero, sizeof box)) {
        static unsigned shown;
        scatter_gather_rw(d, d->regs[NV_PAPU_GPSADDR], d->regs[NV_PAPU_GPSMAXSGE],
                          (uint8_t *)zero, 0x800, sizeof zero, true);
        if (shown++ < 8)
            fprintf(stderr, "[APU] native GP: mailbox %08X %08X cleared\n",
                    *(uint32_t *)box, *(uint32_t *)(box + 4));
    }
}

static int native_gp_interpreted(void)
{
    static int on = -1;
    if (on < 0) {
        const char *e = getenv("RECOMP_AUDIO_GP");
        on = e && *e == '1';
    }
    return on;
}

void mcpx_apu_dsp_frame_interpreted(MCPXAPUState *d, float mixbins[NUM_MIXBINS][NUM_SAMPLES_PER_FRAME])
{
    bool native = mcpx_apu_audio_native() != 0;
    s_gp_frame++;
    gpo_frame(d);
    mcpx_apu_native_mix(d, mixbins);
    /* Native: the EP never runs; its output is the native mix. The GP keeps
     * running: DirectSound loads effect images through its DMA engine and
     * the EA engine paces itself on it. */
    {
        /* RECOMP_AUDIO_GP_DEMAND=1 (experimental): also idle the GP between
         * DirectSound accesses. FIFA Street's engine uses the GP as its audio
         * clock, so with this the intro movie races and the menus freeze. */
        static int demand = -1;
        if (demand < 0) demand = getenv("RECOMP_AUDIO_GP_DEMAND") != NULL;
        if (native && demand && s_gp_frame - s_gp_last_access > 1500)
            return;
    }
    if (native && !native_gp_interpreted()) {
        native_gp_mailbox(d);
        return;
    }
    s_gpo_gp_ran++;
    /* Write VP results to the GP DSP MIXBUF */
    for (int mixbin = 0; mixbin < NUM_MIXBINS; mixbin++) {
        uint32_t base = GP_DSP_MIXBUF_BASE + mixbin * NUM_SAMPLES_PER_FRAME;
        for (int sample = 0; sample < NUM_SAMPLES_PER_FRAME; sample++) {
            dsp_write_memory(d->gp.dsp, 'X', base + sample,
                             float_to_24b(mixbins[mixbin][sample]));
        }
    }

    bool ep_enabled = (d->ep.regs[NV_PAPU_EPRST] & NV_PAPU_GPRST_GPRST) &&
                      (d->ep.regs[NV_PAPU_EPRST] & NV_PAPU_GPRST_GPDSPRST);

    {
        /* RECOMP_APU_BINSTAT=1: per-mixbin RMS of the VP output, every
         * 1500 frames (1 s). */
        static int on = -1;
        static double e[NUM_MIXBINS];
        static unsigned frames;
        if (on < 0) on = getenv("RECOMP_APU_BINSTAT") != NULL;
        if (on) {
            for (int b = 0; b < NUM_MIXBINS; b++)
                for (int i = 0; i < NUM_SAMPLES_PER_FRAME; i++)
                    e[b] += (double)mixbins[b][i] * mixbins[b][i];
            if (++frames % 1500 == 0) {
                fprintf(stderr, "[APU-BINSTAT]");
                for (int b = 0; b < NUM_MIXBINS; b++)
                    if (e[b] > 0)
                        fprintf(stderr, " %d:%.4f", b, sqrt(e[b] / (1500.0 * NUM_SAMPLES_PER_FRAME)));
                fprintf(stderr, "\n");
                memset(e, 0, sizeof e);
            }
        }
    }
    if (d->monitor.point == MCPX_APU_DEBUG_MON_VP) {
        int off = (d->ep_frame_div % 8) * NUM_SAMPLES_PER_FRAME;
        for (int i = 0; i < NUM_SAMPLES_PER_FRAME; i++) {
            float l = mixbins[0][i], r = mixbins[1][i];
            l = l > 1.0f ? 1.0f : l < -1.0f ? -1.0f : l;
            r = r > 1.0f ? 1.0f : r < -1.0f ? -1.0f : r;
            d->monitor.frame_buf[off + i][0] = (int16_t)(l * 32767.0f);
            d->monitor.frame_buf[off + i][1] = (int16_t)(r * 32767.0f);
        }
    }

    /* Run GP */
    {
        extern int mcpx_gpw_enabled(void);
        extern void mcpx_gpw_disarm(void);
        if (mcpx_gpw_enabled()) mcpx_gpw_disarm();
    }
    if ((d->gp.regs[NV_PAPU_GPRST] & NV_PAPU_GPRST_GPRST) &&
        (d->gp.regs[NV_PAPU_GPRST] & NV_PAPU_GPRST_GPDSPRST)) {
        dsp_start_frame(d->gp.dsp);
        dsp_set_halt_requested(d->gp.dsp, false);
        dsp_set_cycle_count(d->gp.dsp, 0);
        do {
            dsp_run(d->gp.dsp, 1000);
        } while (!dsp_get_halt_requested(d->gp.dsp) && d->gp.realtime);
        g_dbg.gp.cycles = dsp_get_cycle_count(d->gp.dsp);

        if ((d->monitor.point == MCPX_APU_DEBUG_MON_GP) ||
            (d->monitor.point == MCPX_APU_DEBUG_MON_GP_OR_EP && !ep_enabled)) {
            int off = (d->ep_frame_div % 8) * NUM_SAMPLES_PER_FRAME;
            for (int i = 0; i < NUM_SAMPLES_PER_FRAME; i++) {
                uint32_t l = dsp_read_memory(d->gp.dsp, 'X', 0x1400 + i);
                d->monitor.frame_buf[off + i][0] = l >> 8;
                uint32_t r =
                    dsp_read_memory(d->gp.dsp, 'X', 0x1400 + 1 * 0x20 + i);
                d->monitor.frame_buf[off + i][1] = r >> 8;
            }
        }
    }

    {   /* RECOMP_GP_WATCH: arm the GP's scratch pages for the guest (apu_gp_watch.c) */
        extern int mcpx_gpw_enabled(void);
        extern void mcpx_gpw_arm(uint8_t *const host[], const uint16_t index[], int n);
        if (mcpx_gpw_enabled() && d->regs[NV_PAPU_GPSADDR]) {
            static uint8_t *host[64];
            static uint16_t idx[64];
            int n = 0;
            for (unsigned pg = 0; pg < 64 && pg <= d->regs[NV_PAPU_GPSMAXSGE]; pg++) {
                uint32_t pa = ldl_le_phys(address_space_memory, d->regs[NV_PAPU_GPSADDR] + pg * 8);
                if (!pa) continue;
                host[n] = mcpx_apu_phys(pa);
                idx[n++] = (uint16_t)pg;
            }
            mcpx_gpw_arm(host, idx, n);
        }
    }

    /* Run EP */
    if (!native && (d->ep.regs[NV_PAPU_EPRST] & NV_PAPU_GPRST_GPRST) &&
        (d->ep.regs[NV_PAPU_EPRST] & NV_PAPU_GPRST_GPDSPRST)) {
        if (d->ep_frame_div % 8 == 0) {
            dsp_start_frame(d->ep.dsp);
            dsp_set_halt_requested(d->ep.dsp, false);
            dsp_set_cycle_count(d->ep.dsp, 0);
            do {
                dsp_run(d->ep.dsp, 1000);
            } while (!dsp_get_halt_requested(d->ep.dsp) && d->ep.realtime);
            g_dbg.ep.cycles = dsp_get_cycle_count(d->ep.dsp);
        }
    }
}

void mcpx_apu_dsp_init_interpreted(MCPXAPUState *d)
{
    d->gp.dsp = dsp_init(d, gp_scratch_rw, gp_fifo_rw, true);
    dsp_set_halt_requested(d->gp.dsp, false);
    dsp_set_cycle_count(d->gp.dsp, 0);

    d->ep.dsp = dsp_init(d, ep_scratch_rw, ep_fifo_rw, false);
    dsp_set_halt_requested(d->ep.dsp, false);
    dsp_set_cycle_count(d->ep.dsp, 0);

    /* Until DSP is more performant, a switch to decide whether or not we should
     * use the full audio pipeline or not.
     */
    d->gp.realtime = true;
    d->ep.realtime = true;
}

uint64_t mcpx_apu_dsp_mmio_read(MCPXAPUState *d, uint64_t addr, unsigned size)
{
    if (!mcpx_apu_dsp_enabled()) return 0;
    if (addr >= 0x30000 && addr < 0x40000) return gp_read(d, addr - 0x30000, size);
    if (addr >= 0x50000 && addr < 0x60000) return ep_read(d, addr - 0x50000, size);
    return 0;
}

void mcpx_apu_dsp_mmio_write(MCPXAPUState *d, uint64_t addr, uint64_t value, unsigned size)
{
    if (!mcpx_apu_dsp_enabled()) return;
    if (addr >= 0x30000 && addr < 0x40000) gp_write(d, addr - 0x30000, value, size);
    if (addr >= 0x50000 && addr < 0x60000) ep_write(d, addr - 0x50000, value, size);
}

void mcpx_apu_dsp_init_passthrough(MCPXAPUState *d);
void mcpx_apu_dsp_frame_passthrough(MCPXAPUState *d, float mixbins[NUM_MIXBINS][NUM_SAMPLES_PER_FRAME]);

int mcpx_apu_dsp_enabled(void)
{
    static int enabled = -1;
    if (enabled < 0) {
        const char *setting = getenv("RECOMP_DSP");
        enabled = setting && *setting && *setting != '0';
    }
    return enabled;
}

void mcpx_apu_dsp_init(MCPXAPUState *d)
{
    if (mcpx_apu_dsp_enabled()) {
        mcpx_apu_dsp_init_interpreted(d);
        fprintf(stderr, "[APU] GP/EP DSP56300 C interpreter initialized\n");
    } else mcpx_apu_dsp_init_passthrough(d);
}

/* RECOMP_APU_MONITOR=vp|gp|ep selects the tap that reaches the speakers
 * (default: EP output FIFO 0, or GP when the EP is not running). */
static int apu_monitor_override(void)
{
    static int point = -2;
    if (point == -2) {
        const char *e = getenv("RECOMP_APU_MONITOR");
        point = -1;
        if (e && !strcmp(e, "vp")) point = MCPX_APU_DEBUG_MON_VP;
        else if (e && !strcmp(e, "gp")) point = MCPX_APU_DEBUG_MON_GP;
        else if (e && !strcmp(e, "ep")) point = MCPX_APU_DEBUG_MON_EP;
    }
    return point;
}

void mcpx_apu_update_dsp_preference(MCPXAPUState *d)
{
    if (mcpx_apu_dsp_enabled()) {
        d->monitor.point = apu_monitor_override() >= 0 ? apu_monitor_override() : MCPX_APU_DEBUG_MON_GP_OR_EP;
        d->gp.realtime = true;
        d->ep.realtime = true;
    }
}

void mcpx_apu_dsp_frame(MCPXAPUState *d, float mixbins[NUM_MIXBINS][NUM_SAMPLES_PER_FRAME])
{
    if (mcpx_apu_dsp_enabled()) mcpx_apu_dsp_frame_interpreted(d, mixbins);
    else mcpx_apu_dsp_frame_passthrough(d, mixbins);
}
