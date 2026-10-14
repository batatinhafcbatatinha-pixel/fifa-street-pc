/* Passive, opt-in evidence capture on the executor thread, before a draw.
 * Addresses are resolved guest addresses, hexadecimal and comma separated.
 * No guest memory is modified. Level zero follows the production span check;
 * complete P8 palettes are preserved rather than composited into a BMP. */
static void texture_snapshot_draw(void)
{
    enum { MAX_SELECTED = 16, MAX_CAPTURES = 128 };
    static struct {
        uint32_t address;
        Texture key;
        uint8_t *raw;
        size_t capacity, bytes;
        uint8_t palette[1024];
        int valid;
    } saved[MAX_SELECTED];
    static unsigned selected, sequence;
    static uint64_t written_bytes;
    static int initialized;
    static char directory[512];
    static char trigger[512];
    if (!initialized) {
        const char *path = getenv("RECOMP_TEX_SNAPSHOT_PATH");
        const char *list = getenv("RECOMP_TEX_SNAPSHOT_ADDRS");
        const char *gate = getenv("RECOMP_TEX_SNAPSHOT_TRIGGER");
        initialized = 1;
        if (!path || !*path || strlen(path) >= sizeof(directory) || !list) return;
        memcpy(directory, path, strlen(path) + 1);
        if (gate && *gate) {
            if (strlen(gate) >= sizeof(trigger)) return;
            memcpy(trigger, gate, strlen(gate) + 1);
        }
        while (*list && selected < MAX_SELECTED) {
            char *end;
            unsigned long long value = strtoull(list, &end, 16);
            if (end == list || value > UINT32_MAX || !value) { selected = 0; return; }
            saved[selected++].address = (uint32_t)value;
            list = end;
            while (*list == ',' || *list == ';' || *list == ' ') ++list;
        }
        if (*list) selected = 0;
    }
    if (!selected || sequence >= MAX_CAPTURES) return;
    /* Wait for an operator-selected frame without consuming any capture
     * slots or byte budget during the intro video. Recheck on each draw;
     * creating the file never changes guest state. */
    if (*trigger && GetFileAttributesA(trigger) == INVALID_FILE_ATTRIBUTES) return;
    const uint8_t *mem = (const uint8_t *)xbox_GetMemoryOffset();
    for (unsigned stage = 0; stage < 4; ++stage) {
        const Texture *t = &s_gpu.texs[stage];
        for (unsigned slot = 0; slot < selected; ++slot) {
            size_t bytes;
            char stem[640], path[680];
            FILE *f;
            if (t->offset != saved[slot].address || !t->valid ||
                !gpu_texture_layout(t, &bytes) || bytes > 16u * 1024u * 1024u) continue;
            int palette = linear_twin(t->color) == 0x0B && t->palette;
            if (saved[slot].valid && saved[slot].bytes == bytes &&
                !memcmp(&saved[slot].key, t, sizeof(*t)) &&
                !memcmp(saved[slot].raw, mem + t->offset, bytes) &&
                (!palette || !memcmp(saved[slot].palette, mem + t->palette, 1024))) continue;
            if (written_bytes + bytes + (palette ? 1024u : 0u) > 256u * 1024u * 1024u) return;
            if (saved[slot].capacity < bytes) {
                void *grown = realloc(saved[slot].raw, bytes);
                if (!grown) continue;
                saved[slot].raw = grown; saved[slot].capacity = bytes;
            }
            /* Freeze bytes before host I/O; all metadata shares this draw cursor. */
            memcpy(saved[slot].raw, mem + t->offset, bytes);
            if (palette) memcpy(saved[slot].palette, mem + t->palette, 1024);
            saved[slot].valid = 0;
            snprintf(stem, sizeof(stem), "%s/texture-%05u-%08X-s%u", directory, sequence, t->offset, stage);
            snprintf(path, sizeof(path), "%s.raw.bin", stem);
            f = fopen(path, "wb");
            if (!f) continue;
            int complete = fwrite(saved[slot].raw, 1, bytes, f) == bytes;
            if (fclose(f)) complete = 0;
            if (palette) {
                snprintf(path, sizeof(path), "%s.palette.bin", stem);
                f = fopen(path, "wb");
                if (!f) complete = 0;
                else { if (fwrite(saved[slot].palette, 1, 1024, f) != 1024) complete = 0; if (fclose(f)) complete = 0; }
            }
            Nv2aVshState vsh;
            nv2a_vsh_snapshot(&vsh);
            Nv2aVshInput *inputs = (Nv2aVshInput *)malloc(s_gpu.idx_count * sizeof(*inputs));
            snprintf(path, sizeof(path), "%s.inputs.bin", stem);
            if (!inputs) complete = 0;
            else {
                for (unsigned k = 0; k < s_gpu.idx_count; ++k)
                    for (unsigned a = 0; a < NV2A_VSH_INPUTS; ++a)
                        fetch_attr(&s_gpu.attr[a], s_gpu.idx[k], inputs[k].attr[a]);
                f = fopen(path, "wb");
                if (!f) complete = 0;
                else { if (fwrite(inputs, sizeof(*inputs), s_gpu.idx_count, f) != s_gpu.idx_count) complete = 0; if (fclose(f)) complete = 0; }
                free(inputs);
            }
            snprintf(path, sizeof(path), "%s.vsh.bin", stem);
            f = fopen(path, "wb");
            if (!f) complete = 0;
            else { if (fwrite(&vsh, 1, sizeof(vsh), f) != sizeof(vsh)) complete = 0; if (fclose(f)) complete = 0; }
            snprintf(path, sizeof(path), "%s.json", stem);
            f = fopen(path, "wb");
            if (!f) complete = 0;
            else {
                fprintf(f, "{\n\"draw\":%u,\"flip\":%u,\"stage\":%u,\"primitive\":%u,\"xf_mode\":%u,\"vsh_start\":%u,\"cxt_write\":%u,\n",
                        s_gpu.draws, s_gpu.flips, stage, s_gpu.prim, s_gpu.xf_mode, vsh.start, vsh.cxt_write);
                fprintf(f, "\"texture\":{\"address\":%u,\"bytes\":%zu,\"width\":%u,\"height\":%u,\"pitch\":%u,\"format\":%u,\"palette\":%u,\"palette_bytes\":%u,\"filter\":%u,\"addr_u\":%u,\"addr_v\":%u,\"levels\":%u,\"cube\":%d},\n",
                        t->offset, bytes, t->width, t->height, t->pitch, t->color, t->palette, palette ? 1024u : 0u, t->filter, t->addr_u, t->addr_v, t->levels, t->cube);
                fprintf(f, "\"surface\":{\"address\":%u,\"resolved\":%u,\"format\":%u,\"pitch\":%u,\"clip\":[%u,%u,%u,%u],\"samples\":[%u,%u]},\n",
                        s_gpu.color_offset, dma_resolve(s_gpu.color_offset), s_gpu.format, s_gpu.pitch, s_gpu.clip_x, s_gpu.clip_y, s_gpu.clip_w, s_gpu.clip_h, surface_samples_x(), surface_samples_y());
                fprintf(f, "\"blend\":[%u,%u,%u,%u,%u],\"color_mask\":%u,\"alpha\":[%u,%u,%u],\"depth\":[%u,%u,%u],\n",
                        s_gpu.blend_enable, s_gpu.blend_sfactor, s_gpu.blend_dfactor, s_gpu.blend_equation, s_gpu.blend_color, s_gpu.color_mask, s_gpu.alpha_test, s_gpu.alpha_func, s_gpu.alpha_ref, s_gpu.depth_test, s_gpu.depth_func, s_gpu.depth_mask);
                fprintf(f, "\"combiner\":{\"seen\":%d,\"control\":%u,\"stage_program\":%u,\"final\":[%u,%u,%u,%u],\"stages\":[",
                        s_gpu.rc_seen, s_gpu.rc.control, s_gpu.rc.stage_program, s_gpu.rc.final0, s_gpu.rc.final1, s_gpu.rc.final_c0, s_gpu.rc.final_c1);
                for (unsigned k = 0; k < 8; ++k) fprintf(f, "%s[%u,%u,%u,%u,%u,%u]", k ? "," : "", s_gpu.rc.color_icw[k], s_gpu.rc.alpha_icw[k], s_gpu.rc.color_ocw[k], s_gpu.rc.alpha_ocw[k], s_gpu.rc.factor0[k], s_gpu.rc.factor1[k]);
                fprintf(f, "]},\"indices\":[");
                for (unsigned k = 0; k < s_gpu.idx_count; ++k) fprintf(f, "%s%u", k ? "," : "", s_gpu.idx[k]);
                fprintf(f, "],\"attributes\":[");
                for (unsigned k = 0; k < 16; ++k) fprintf(f, "%s[%u,%u,%u,%u]", k ? "," : "", s_gpu.attr[k].offset, s_gpu.attr[k].type, s_gpu.attr[k].size, s_gpu.attr[k].stride);
                fprintf(f, "]}\n");
                if (ferror(f)) complete = 0;
                if (fclose(f)) complete = 0;
            }
            ++sequence; written_bytes += bytes + (palette ? 1024u : 0u);
            saved[slot].key = *t; saved[slot].bytes = bytes; saved[slot].valid = complete;
            fprintf(stderr, "[TEX-SNAPSHOT] draw=%u flip=%u stage=%u %08X bytes=%zu palette=%u complete=%d %s\n", s_gpu.draws, s_gpu.flips, stage, t->offset, bytes, palette ? 1024u : 0u, complete, stem);
        }
    }
}
