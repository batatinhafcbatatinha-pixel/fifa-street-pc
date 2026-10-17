/*
 * Controller input in C: the XDK XInput/XPP API without the USB stack.
 *
 * The game links XAPI's XPP layer, which drives the console's OHCI USB host
 * controller and an XID driver; the runtime answers that with a register
 * model of the controller (xboxrecomp/src/usb/ohci.c) and an emulated pad on
 * its bus (usb_gamepad.c). With RECOMP_INPUT_NATIVE=1 (launcher default) the
 * functions below replace the API itself -- routed by hle/input_functions.json
 * through hle/apply-hle-wrappers.py -- so XInitDevices never starts the USB
 * driver, main.c skips the OHCI model and no USB interrupt, DPC or timer runs.
 *
 * The pad state still comes from usb_gamepad_report(): host XInput / SDL pad n,
 * the keyboard on pad 0 (RECOMP_KEYBOARD) and the test scripts
 * (RECOMP_PAD_SCRIPT / RECOMP_PAD_LIVE / RECOMP_PAD_PRESS). Its 20-byte report
 * is the XID report the original driver copied: bytes 2..19 are XINPUT_GAMEPAD.
 *
 * Ports: port 0 is always connected (keyboard and scripts drive it, as the
 * one emulated pad did); ports 1-3 follow host pads 1-3, plus every port below
 * RECOMP_USB_PADS. Memory units: none (the MU device type reports nothing, so
 * the game never calls XMountMUA).
 *
 * Device-type objects (g_DeviceType_Gamepad 0x26F274, _MU 0x26F320) keep the
 * library's layout -- +0 connected, +4 changed, +8 previous -- and the C
 * XGetDevices/XGetDeviceChanges do the same bit arithmetic as the library.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "hle_win32.h"

int xbox_no_hardware(void);

void sub_0026F733_gen(void);   /* XInitDevices(count, types) */
void sub_002704D9_gen(void);   /* XGetDevices(type) */
void sub_002704FB_gen(void);   /* XGetDeviceChanges(type, ins, rem) */
void sub_002701D2_gen(void);   /* XInputOpen(type, port, slot, poll) */
void sub_00270228_gen(void);   /* XInputClose(h) */
void sub_00270234_gen(void);   /* XInputGetCapabilities(h, caps) */
void sub_0027040C_gen(void);   /* XInputGetState(h, state) */
void sub_0027047F_gen(void);   /* XInputSetState(h, feedback) */

/* xboxrecomp/src/usb/usb_gamepad.c and src/input/xinput_device.c */
int usb_gamepad_report(int pad, uint8_t *out, int max);
void xbox_InputInit(void);
BOOL xbox_InputIsConnected(DWORD port);
typedef struct { WORD left, right; } HostVibration;
DWORD xbox_InputSetState(DWORD port, const void *vibration);

#define TYPE_GAMEPAD  0x0026F274u
#define TYPE_MU       0x0026F320u
#define NPORTS        4
#define HANDLE_BASE   0x00C0DE00u        /* opaque; only these functions read it */

#define ERR_SUCCESS            0u
#define ERR_IO_PENDING         997u
#define ERR_NOT_CONNECTED      1167u

typedef struct {
    int open;
    uint32_t packet;
    uint8_t last[18];                    /* XINPUT_GAMEPAD as last reported */
    int have_last;
    WORD rumble_l, rumble_r;
} Port;

static Port s_port[NPORTS];
static int s_native = -1;
static int s_inited;
static int s_min_pads = 1;               /* RECOMP_USB_PADS */
static int s_log;
static uint32_t s_host_mask;             /* ports 1-3 with a host pad */
static DWORD s_host_probe_tick;
static uint64_t s_calls[8];

int hle_input_native_enabled(void)
{
    if (s_native < 0) {
        const char *e = getenv("RECOMP_INPUT_NATIVE");
        s_native = e && *e && *e != '0';
    }
    return s_native;
}

static void input_init(void)
{
    const char *e;
    if (s_inited)
        return;
    s_inited = 1;
    e = getenv("RECOMP_USB_PADS");
    if (e && atoi(e) >= 1 && atoi(e) <= NPORTS)
        s_min_pads = atoi(e);
    e = getenv("RECOMP_INPUT_LOG");
    s_log = e ? atoi(e) : 0;
    xbox_InputInit();
    fprintf(stderr, "[INPUT-NATIVE] XInput API in C: no USB stack, %d port(s) always connected\n",
            s_min_pads);
}

/* Host pads behind ports 1-3. An empty XInput slot is slow to ask about, so
 * the probe runs at most twice a second; the game asks every frame. */
static uint32_t connected_mask(void)
{
    DWORD now = GetTickCount();
    uint32_t mask = (1u << s_min_pads) - 1u;
    if (!s_host_probe_tick || now - s_host_probe_tick >= 500) {
        uint32_t m = 0;
        int i;
        uint8_t rep[20];
        s_host_probe_tick = now ? now : 1;
        for (i = 1; i < NPORTS; i++) {
            usb_gamepad_report(i, rep, 20);     /* refreshes the connected flag */
            if (xbox_InputIsConnected((DWORD)i))
                m |= 1u << i;
        }
        if (m != s_host_mask)
            fprintf(stderr, "[INPUT-NATIVE] host pads on ports 1-3: mask %X\n", m);
        s_host_mask = m;
    }
    return mask | s_host_mask;
}

static uint32_t type_mask(uint32_t type)
{
    return type == TYPE_GAMEPAD ? connected_mask() : 0;
}

static int handle_port(uint32_t h)
{
    uint32_t p = h - HANDLE_BASE - 1u;
    return p < NPORTS && s_port[p].open ? (int)p : -1;
}

/* stdcall return: value in eax, pop the return address and the arguments. */
static void ret_stdcall(uint32_t value, int nargs)
{
    g_eax = value;
    g_esp += 4u + 4u * (uint32_t)nargs;
}

int guest_cpu_native_enter(void);
void guest_cpu_native_leave(int token);

#define ARG(i) MEM32(g_esp + 4u + 4u * (i))

static void input_0026F733(void)
{
    if (!hle_input_native_enabled()) {
        sub_0026F733_gen();
        return;
    }
    input_init();
    s_calls[0]++;
    {
        uint32_t n = ARG(0), types = ARG(1), i;
        for (i = 0; i < n && types && i < 8; i++)
            fprintf(stderr, "[INPUT-NATIVE] XInitDevices type %08X prealloc %u\n",
                    MEM32(types + 8 * i), MEM32(types + 8 * i + 4));
    }
    /* Nothing is connected until the title looks: XGetDevices then reports
     * the current set, and XGetDeviceChanges reports it as insertions. */
    MEM32(TYPE_GAMEPAD + 0) = 0; MEM32(TYPE_GAMEPAD + 4) = 0; MEM32(TYPE_GAMEPAD + 8) = 0;
    MEM32(TYPE_MU + 0) = 0; MEM32(TYPE_MU + 4) = 0; MEM32(TYPE_MU + 8) = 0;
    ret_stdcall(0, 2);
}

/* The library's +0/+4/+8 bookkeeping, fed by the host instead of the USB
 * hub driver. (+4, "went away and came back between two polls", is never
 * set: the host is sampled at poll time.) */
static void refresh_type(uint32_t type)
{
    MEM32(type) = type_mask(type);
}

static void input_002704D9(void)
{
    uint32_t type, cur;
    if (!hle_input_native_enabled()) {
        sub_002704D9_gen();
        return;
    }
    input_init();
    s_calls[1]++;
    type = ARG(0);
    refresh_type(type);
    cur = MEM32(type);
    MEM32(type + 4) = 0;
    MEM32(type + 8) = cur;
    ret_stdcall(cur, 1);
}

static void input_002704FB(void)
{
    uint32_t type, pins, prem, cur, chg, prev, ins, rem, both;
    if (!hle_input_native_enabled()) {
        sub_002704FB_gen();
        return;
    }
    input_init();
    s_calls[2]++;
    type = ARG(0); pins = ARG(1); prem = ARG(2);
    refresh_type(type);
    cur = MEM32(type); chg = MEM32(type + 4); prev = MEM32(type + 8);
    both = chg & prev & cur;
    ins = (cur & ~prev) | both;
    rem = (prev & ~cur) | both;
    MEM32(pins) = ins;
    MEM32(prem) = rem;
    MEM32(type + 4) = 0;
    MEM32(type + 8) = cur;
    if (ins || rem)
        fprintf(stderr, "[INPUT-NATIVE] XGetDeviceChanges %s: inserted %X removed %X\n",
                type == TYPE_GAMEPAD ? "gamepad" : type == TYPE_MU ? "MU" : "?", ins, rem);
    ret_stdcall((ins | rem) ? 1u : 0u, 3);
}

static void input_002701D2(void)
{
    uint32_t type, port, slot, h = 0;
    if (!hle_input_native_enabled()) {
        sub_002701D2_gen();
        return;
    }
    input_init();
    s_calls[3]++;
    type = ARG(0); port = ARG(1); slot = ARG(2);
    if (type == TYPE_GAMEPAD && port < NPORTS && slot == 0 && (type_mask(type) >> port & 1u)) {
        Port *p = &s_port[port];
        memset(p, 0, sizeof *p);
        p->open = 1;
        h = HANDLE_BASE + 1u + port;
    }
    fprintf(stderr, "[INPUT-NATIVE] XInputOpen type %08X port %u slot %u -> %08X\n", type, port, slot, h);
    ret_stdcall(h, 4);
}

static void input_00270228(void)
{
    int port;
    if (!hle_input_native_enabled()) {
        sub_00270228_gen();
        return;
    }
    s_calls[4]++;
    port = handle_port(ARG(0));
    if (port >= 0) {
        HostVibration v = { 0, 0 };
        if (s_port[port].rumble_l || s_port[port].rumble_r)
            xbox_InputSetState((DWORD)port, &v);
        s_port[port].open = 0;
        fprintf(stderr, "[INPUT-NATIVE] XInputClose port %d\n", port);
    }
    ret_stdcall(0, 1);
}

/* XINPUT_CAPABILITIES is packed, 25 bytes: SubType @0, Reserved @1,
 * In.Gamepad @3 (18 bytes), Out.Rumble @21 (library 0x270234 zeroes 25 bytes,
 * lands the two GET_CAPABILITIES reports at +1 and +0x13, then clears +1).
 * Values: the emulated pad's answers -- every field supported, XID bSubType
 * 2 (gamepad "S"); wButtons' mask is 0x00FF. */
static void input_00270234(void)
{
    uint32_t caps, i;
    int port;
    if (!hle_input_native_enabled()) {
        sub_00270234_gen();
        return;
    }
    s_calls[5]++;
    port = handle_port(ARG(0));
    caps = ARG(1);
    if (port < 0) {
        ret_stdcall(ERR_NOT_CONNECTED, 2);
        return;
    }
    for (i = 0; i < 25; i++)
        MEM8(caps + i) = 0;
    MEM8(caps + 0) = 2;
    MEM8(caps + 3) = 0xFF;
    for (i = 5; i < 25; i++)
        MEM8(caps + i) = 0xFF;
    ret_stdcall(ERR_SUCCESS, 2);
}

/* XINPUT_STATE: dwPacketNumber, then XINPUT_GAMEPAD = report bytes 2..19.
 * The packet number moves only when the state does, as on the console. */
static void input_0027040C(void)
{
    uint8_t rep[20];
    uint32_t st, i;
    int port;
    Port *p;
    if (!hle_input_native_enabled()) {
        sub_0027040C_gen();
        return;
    }
    s_calls[6]++;
    port = handle_port(ARG(0));
    st = ARG(1);
    if (port < 0 || !(connected_mask() >> port & 1u)) {
        ret_stdcall(ERR_NOT_CONNECTED, 2);
        return;
    }
    p = &s_port[port];
    /* Nothing should touch the USB controllers' registers now (plain memory
     * without the OHCI model): check once in a while and say so if it does.
     * In a hardware-free run the aperture is not mapped at all and the
     * access guard reports any touch, so reading it here would be one. */
    if (s_calls[6] % 3600 == 1 && !xbox_no_hardware()) {
        static int reported;
        uint32_t off, bad = 0;
        for (off = 0; off < 0x1000 && !bad; off += 4)
            if (MEM32(0xFED00000u + off) || MEM32(0xFED08000u + off))
                bad = 0xFED00000u + off;
        if (bad && !reported++)
            fprintf(stderr, "[INPUT-NATIVE] WARNING: OHCI registers written (%08X) - USB stack running?\n", bad);
    }
    usb_gamepad_report(port, rep, 20);
    if (!p->have_last || memcmp(p->last, rep + 2, 18)) {
        memcpy(p->last, rep + 2, 18);
        p->have_last = 1;
        p->packet++;
        if (s_log > 0) {
            s_log--;
            fprintf(stderr, "[INPUT-NATIVE] port %d packet %u buttons %02X analog %02X %02X %02X %02X"
                    " LX %d LY %d\n", port, p->packet, rep[2], rep[4], rep[5], rep[6], rep[7],
                    (int16_t)(rep[12] | rep[13] << 8), (int16_t)(rep[14] | rep[15] << 8));
        }
    }
    MEM32(st) = p->packet;
    for (i = 0; i < 18; i++)
        MEM8(st + 4 + i) = p->last[i];
    ret_stdcall(ERR_SUCCESS, 2);
}

/* XINPUT_FEEDBACK: header { dwStatus, hEvent, reserved[58] }, rumble @66.
 * Completes at once: the title waits for dwStatus to leave IO_PENDING. */
static void input_0027047F(void)
{
    uint32_t fb, ev;
    int port;
    if (!hle_input_native_enabled()) {
        sub_0027047F_gen();
        return;
    }
    s_calls[7]++;
    port = handle_port(ARG(0));
    fb = ARG(1);
    if (port < 0) {
        ret_stdcall(ERR_NOT_CONNECTED, 2);
        return;
    }
    {
        Port *p = &s_port[port];
        HostVibration v;
        v.left = MEM16(fb + 66);
        v.right = MEM16(fb + 68);
        if (v.left != p->rumble_l || v.right != p->rumble_r) {
            p->rumble_l = v.left;
            p->rumble_r = v.right;
            xbox_InputSetState((DWORD)port, &v);
        }
    }
    ev = MEM32(fb + 4);
    if (ev) {
        static int warned;
        if (!warned++)
            fprintf(stderr, "[INPUT-NATIVE] XInputSetState with a completion event %08X (not signalled)\n", ev);
    }
    MEM32(fb) = ERR_SUCCESS;
    ret_stdcall(ERR_IO_PENDING, 2);
}

/* Entry points: under RECOMP_GUEST_SMP the API runs under the guest kernel lock. */
void sub_0026F733(void) { int s = guest_cpu_native_enter(); input_0026F733(); guest_cpu_native_leave(s); }
void sub_002704D9(void) { int s = guest_cpu_native_enter(); input_002704D9(); guest_cpu_native_leave(s); }
void sub_002704FB(void) { int s = guest_cpu_native_enter(); input_002704FB(); guest_cpu_native_leave(s); }
void sub_002701D2(void) { int s = guest_cpu_native_enter(); input_002701D2(); guest_cpu_native_leave(s); }
void sub_00270228(void) { int s = guest_cpu_native_enter(); input_00270228(); guest_cpu_native_leave(s); }
void sub_00270234(void) { int s = guest_cpu_native_enter(); input_00270234(); guest_cpu_native_leave(s); }
void sub_0027040C(void) { int s = guest_cpu_native_enter(); input_0027040C(); guest_cpu_native_leave(s); }
void sub_0027047F(void) { int s = guest_cpu_native_enter(); input_0027047F(); guest_cpu_native_leave(s); }
