/*
 * Hooks on the game's own code (routed by hle/game_hooks.json through
 * hle/apply-hle-wrappers.py: the generated body becomes sub_X_gen).
 *
 * The front end is Flash (EA's APT). Its screens reach the game through
 * native classes registered by name (registration at 0x0A7540..). The
 * "options" class (constructor 0x8D060) backs the Options submenus: its init
 * method (0x8D0A0) receives the screen's context string, matches it against
 * "AUDIO", "VIDEO", "EATRAX" (pointers at 0x2BF380) and stores the index at
 * 0x2BF3EC, picking the title "Audio Options" / "Video Options". Index 1 is
 * the player opening Options > Video, from the main menu or the pause menu:
 * that is where the PC settings screen (hle_options.c) opens.
 */
#include <stdio.h>
#include "recomp/gen/recomp_types.h"

#define OPTIONS_CONTEXT_INDEX 0x2BF3ECu        /* 0 audio, 1 video, 2 EA Trax */

void sub_0008D0A0_gen(void);
void hle_options_request(const char *why);

void sub_0008D0A0(void)
{
    /* The body writes the index only when the context matches; a marker
     * tells a match from a context it does not know. */
    uint32_t before = MEM32(OPTIONS_CONTEXT_INDEX), index;
    MEM32(OPTIONS_CONTEXT_INDEX) = 0xFFFFFFFFu;
    sub_0008D0A0_gen();
    index = MEM32(OPTIONS_CONTEXT_INDEX);
    if (index == 0xFFFFFFFFu) {
        MEM32(OPTIONS_CONTEXT_INDEX) = before;
        return;
    }
    fprintf(stderr, "[GAME] Options screen %u (%s)\n", index,
            index == 0 ? "audio" : index == 1 ? "video" : "EA Trax");
    if (index == 1)
        hle_options_request("the game's Options > Video");
}

/* Front-end screen loads: 0xA2BB0(const char *name) builds "<name>.big". */
void sub_000A2BB0_gen(void);
void game_screen_loaded(const char *name);

void sub_000A2BB0(void)
{
    char name[48];
    uint32_t p = MEM32(g_esp + 4);
    int i;
    for (i = 0; i < 47 && p && MEM8(p + (uint32_t)i); i++)
        name[i] = (char)MEM8(p + (uint32_t)i);
    name[i] = 0;
    sub_000A2BB0_gen();
    game_screen_loaded(name);
}
