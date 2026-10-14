/*
 * mmio_decode.h -- one x86-64 instruction decoder for trapped MMIO.
 *
 * A device whose registers need semantics cannot be plain memory: the reads
 * have to be answered and the writes have to be seen. The way that works here
 * is to leave the page PAGE_NOACCESS, catch the access in a vectored handler,
 * decode the faulting instruction, service it against the device model, and
 * step over it.
 *
 * That decoder existed twice before this file -- once in nv2a_mmio_hook.c and
 * once in apu_mmio_hook.c -- as the same opcode table written out against two
 * different pairs of accessors. This is the same logic with the accessors
 * passed in, so a third device does not need a third copy. The two originals
 * still carry their own and can move onto this whenever they are next touched.
 *
 * Header-only and static inline: one function, one caller per device, and a
 * library for it would be more build wiring than code.
 *
 * The instructions covered are what the XDK device code actually emits against
 * registers -- moves both ways, the immediate forms, the zero-extending loads,
 * and the read-modify-write and flag-setting forms a poll loop is built from.
 * Anything outside that set returns 0 rather than guessing: stepping over an
 * instruction that was not understood corrupts the guest silently, which is
 * far worse than a fault naming the opcode.
 */
#ifndef MMIO_DECODE_H
#define MMIO_DECODE_H

#include <stdint.h>

/* Any x86-64 host: on POSIX the vectored handlers run over sigaction
 * (win32_compat.c) with the same CONTEXT. */
#if defined(_WIN32) || defined(__x86_64__)
#if defined(_WIN32)
#include <windows.h>
#else
#include "platform/xbox_winnt.h"
#include "platform/win32_compat.h"
#endif

/* Service one register access. dev is passed straight back to the callbacks. */
typedef uint64_t (*mmio_read_fn)(void *dev, uint32_t off, int size);
typedef void     (*mmio_write_fn)(void *dev, uint32_t off, uint64_t val, int size);

static inline uint64_t *mmio_ctx_reg(PCONTEXT c, int reg)
{
    switch (reg & 0xF) {
    case 0:  return (uint64_t *)&c->Rax;   case 1:  return (uint64_t *)&c->Rcx;
    case 2:  return (uint64_t *)&c->Rdx;   case 3:  return (uint64_t *)&c->Rbx;
    case 4:  return (uint64_t *)&c->Rsp;   case 5:  return (uint64_t *)&c->Rbp;
    case 6:  return (uint64_t *)&c->Rsi;   case 7:  return (uint64_t *)&c->Rdi;
    case 8:  return (uint64_t *)&c->R8;    case 9:  return (uint64_t *)&c->R9;
    case 10: return (uint64_t *)&c->R10;   case 11: return (uint64_t *)&c->R11;
    case 12: return (uint64_t *)&c->R12;   case 13: return (uint64_t *)&c->R13;
    case 14: return (uint64_t *)&c->R14;   case 15: return (uint64_t *)&c->R15;
    default: return NULL;
    }
}

static inline int mmio_modrm_len(const uint8_t *ip, int rex_b)
{
    uint8_t modrm = *ip;
    int mod = (modrm >> 6) & 3;
    int rm  = (modrm & 7) | (rex_b ? 8 : 0);
    int len = 1;

    if (mod == 3) return 1;
    if ((rm & 7) == 4) len += 1;                    /* SIB    */
    if (mod == 0 && (rm & 7) == 5) len += 4;        /* disp32 */
    else if (mod == 1) len += 1;
    else if (mod == 2) len += 4;
    return len;
}

/* Flags after a compare-shaped operation, so a poll loop branches correctly.
 * ZF, SF and CF only: those are what jz/jnz, js and jb/jae read, and inventing
 * an overflow flag nothing here sets is worse than leaving it alone. */
static inline void mmio_set_flags(PCONTEXT ctx, uint64_t result, int size,
                                  int carry)
{
    ctx->EFlags &= ~(0x0001u | 0x0040u | 0x0080u | 0x0800u);
    if (size < 8)
        result &= (1ULL << (size * 8)) - 1;
    if (result == 0)
        ctx->EFlags |= 0x0040u;                                   /* ZF */
    if (result & (1ULL << (size * 8 - 1)))
        ctx->EFlags |= 0x0080u;                                   /* SF */
    if (carry)
        ctx->EFlags |= 0x0001u;                                   /* CF */
}

/* 1 if the instruction at ctx->Rip was serviced and Rip advanced past it. */
static inline int mmio_emulate(PCONTEXT ctx, uint32_t off, void *dev,
                               mmio_read_fn rd, mmio_write_fn wr)
{
    const uint8_t *ip = (const uint8_t *)ctx->Rip;
    int prefix = 0, has66 = 0, rex = 0, has_rex = 0;
    const uint8_t *op;
    int size, rex_w, rex_r, rex_b, mlen, reg;

    if (!rd || !wr)
        return 0;

    for (;;) {
        uint8_t b = ip[prefix];
        if (b == 0x66)                   { has66 = 1; prefix++; }
        else if (b == 0xF2 || b == 0xF3) { prefix++; }
        else if (b >= 0x40 && b <= 0x4F) { rex = b; has_rex = 1; prefix++; }
        else break;
    }
    rex_w = has_rex && (rex & 0x08);
    rex_r = has_rex && (rex & 0x04);
    rex_b = has_rex && (rex & 0x01);

    op   = ip + prefix;
    size = has66 ? 2 : (rex_w ? 8 : 4);

    switch (op[0]) {
    case 0x88: case 0x89:                            /* MOV r/m, r   (write) */
        if (op[0] == 0x88) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        wr(dev, off, *mmio_ctx_reg(ctx, reg), size);
        ctx->Rip += prefix + 1 + mlen;
        return 1;

    case 0xC7:                                       /* MOV r/m, imm32       */
        mlen = mmio_modrm_len(op + 1, rex_b);
        wr(dev, off, *(const uint32_t *)(op + 1 + mlen), size);
        ctx->Rip += prefix + 1 + mlen + 4;
        return 1;

    case 0xC6:                                       /* MOV r/m8, imm8       */
        mlen = mmio_modrm_len(op + 1, rex_b);
        wr(dev, off, op[1 + mlen], 1);
        ctx->Rip += prefix + 1 + mlen + 1;
        return 1;

    case 0x8A: case 0x8B: {                          /* MOV r, r/m   (read)  */
        uint64_t v, *dst;
        if (op[0] == 0x8A) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        v    = rd(dev, off, size);
        dst  = mmio_ctx_reg(ctx, reg);
        if (size == 1)      *dst = (*dst & ~0xFFULL)   | (v & 0xFF);
        else if (size == 2) *dst = (*dst & ~0xFFFFULL) | (v & 0xFFFF);
        else if (size == 4) *dst = v & 0xFFFFFFFFULL;  /* 32-bit clears high */
        else                *dst = v;
        ctx->Rip += prefix + 1 + mlen;
        return 1;
    }

    case 0x84: case 0x85: {                          /* TEST r/m, r          */
        uint64_t m, r;
        if (op[0] == 0x84) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        m    = rd(dev, off, size);
        r    = *mmio_ctx_reg(ctx, reg);
        mmio_set_flags(ctx, m & r, size, 0);
        ctx->Rip += prefix + 1 + mlen;
        return 1;
    }

    case 0x38: case 0x39: {                          /* CMP r/m, r           */
        uint64_t m, r, mask;
        if (op[0] == 0x38) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        m    = rd(dev, off, size);
        r    = *mmio_ctx_reg(ctx, reg);
        mask = (size < 8) ? ((1ULL << (size * 8)) - 1) : ~0ULL;
        m &= mask; r &= mask;
        mmio_set_flags(ctx, m - r, size, m < r);
        ctx->Rip += prefix + 1 + mlen;
        return 1;
    }

    case 0x08: case 0x09:                            /* OR  r/m, r           */
        if (op[0] == 0x08) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        wr(dev, off, rd(dev, off, size) | *mmio_ctx_reg(ctx, reg), size);
        ctx->Rip += prefix + 1 + mlen;
        return 1;

    case 0x20: case 0x21:                            /* AND r/m, r           */
        if (op[0] == 0x20) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        wr(dev, off, rd(dev, off, size) & *mmio_ctx_reg(ctx, reg), size);
        ctx->Rip += prefix + 1 + mlen;
        return 1;

    /* ADD/OR/AND/SUB/XOR/CMP r, r/m: the register reads the device. GCC and
     * clang on Linux fold a register read into the operation that uses it
     * ("and edx, [rax+rcx]") where clang-cl kept a separate mov. */
    case 0x02: case 0x03: case 0x0A: case 0x0B: case 0x22: case 0x23:
    case 0x2A: case 0x2B: case 0x32: case 0x33: case 0x3A: case 0x3B: {
        uint64_t m, r, res, mask, *dst;
        int kind = op[0] >> 3;                       /* 0 add 1 or 4 and 5 sub 6 xor 7 cmp */
        if (!(op[0] & 1)) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        reg  = ((op[1] >> 3) & 7) | (rex_r ? 8 : 0);
        dst  = mmio_ctx_reg(ctx, reg);
        mask = (size < 8) ? ((1ULL << (size * 8)) - 1) : ~0ULL;
        m    = rd(dev, off, size) & mask;
        r    = *dst & mask;
        switch (kind) {
        case 0: res = r + m; break;
        case 1: res = r | m; break;
        case 4: res = r & m; break;
        case 6: res = r ^ m; break;
        default: res = r - m; break;                 /* sub, cmp */
        }
        mmio_set_flags(ctx, res, size, kind == 0 ? ((res & mask) < r) : (kind == 5 || kind == 7) ? r < m : 0);
        if (kind != 7) {
            res &= mask;
            if (size == 1)      *dst = (*dst & ~0xFFULL)   | res;
            else if (size == 2) *dst = (*dst & ~0xFFFFULL) | res;
            else                *dst = res;          /* 32-bit clears high */
        }
        ctx->Rip += prefix + 1 + mlen;
        return 1;
    }

    /* ADD/OR/AND/SUB/XOR/CMP r/m, imm: read-modify-write of the register
     * (CMP only reads). 0x80 imm8 on a byte, 0x81 imm32, 0x83 imm8 sign-extended. */
    case 0x80: case 0x81: case 0x83: {
        uint64_t m, imm, res, mask;
        int kind, isz;
        if (op[0] == 0x80) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        kind = (op[1] >> 3) & 7;
        if (kind == 2 || kind == 3)                  /* adc/sbb: not emitted against registers */
            return 0;
        mask = (size < 8) ? ((1ULL << (size * 8)) - 1) : ~0ULL;
        if (op[0] == 0x81) { imm = size == 2 ? *(const uint16_t *)(op + 1 + mlen) : (uint64_t)(int64_t)*(const int32_t *)(op + 1 + mlen); isz = size == 2 ? 2 : 4; }
        else               { imm = (uint64_t)(int64_t)(int8_t)op[1 + mlen]; isz = 1; }
        imm &= mask;
        m = rd(dev, off, size) & mask;
        switch (kind) {
        case 0: res = m + imm; break;
        case 1: res = m | imm; break;
        case 4: res = m & imm; break;
        case 6: res = m ^ imm; break;
        default: res = m - imm; break;
        }
        mmio_set_flags(ctx, res, size, kind == 0 ? ((res & mask) < m) : (kind == 5 || kind == 7) ? m < imm : 0);
        if (kind != 7)
            wr(dev, off, res & mask, size);
        ctx->Rip += prefix + 1 + mlen + isz;
        return 1;
    }

    case 0xF6: case 0xF7:                            /* TEST r/m, imm        */
        if (op[0] == 0xF6) size = 1;
        mlen = mmio_modrm_len(op + 1, rex_b);
        if (((op[1] >> 3) & 7) != 0)
            return 0;
        {
            uint64_t imm = size == 1 ? op[1 + mlen] : size == 2 ? *(const uint16_t *)(op + 1 + mlen)
                                                                : (uint64_t)*(const uint32_t *)(op + 1 + mlen);
            mmio_set_flags(ctx, rd(dev, off, size) & imm, size, 0);
            ctx->Rip += prefix + 1 + mlen + (size == 1 ? 1 : size == 2 ? 2 : 4);
        }
        return 1;

    case 0x0F:
        if (op[1] == 0xBE || op[1] == 0xBF) {        /* MOVSX r32, r/m8|16   */
            int s = (op[1] == 0xBE) ? 1 : 2;
            int64_t v;
            mlen  = mmio_modrm_len(op + 2, rex_b);
            reg   = ((op[2] >> 3) & 7) | (rex_r ? 8 : 0);
            v     = s == 1 ? (int8_t)rd(dev, off, 1) : (int16_t)rd(dev, off, 2);
            *mmio_ctx_reg(ctx, reg) = rex_w ? (uint64_t)v : (uint64_t)(uint32_t)v;
            ctx->Rip += prefix + 2 + mlen;
            return 1;
        }
        if (op[1] == 0xB6 || op[1] == 0xB7) {        /* MOVZX r32, r/m8|16   */
            int s = (op[1] == 0xB6) ? 1 : 2;
            mlen  = mmio_modrm_len(op + 2, rex_b);
            reg   = ((op[2] >> 3) & 7) | (rex_r ? 8 : 0);
            *mmio_ctx_reg(ctx, reg) = rd(dev, off, s) & ((1ULL << (s * 8)) - 1);
            ctx->Rip += prefix + 2 + mlen;
            return 1;
        }
        return 0;

    default:
        return 0;
    }
}

#endif /* _WIN32 */
#endif /* MMIO_DECODE_H */
