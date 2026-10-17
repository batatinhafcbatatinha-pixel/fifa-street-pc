static void load_xbe(const char *path)
{
    FILE *f = fopen(path, "rb");
    assert(f);
    uint32_t header[0x180 / 4];
    assert(fread(header, 1, sizeof(header), f) == sizeof(header));
    assert(header[0] == 0x48454258u);
    uint32_t base = header[0x104 / 4], count = header[0x11C / 4];
    uint32_t sections = header[0x120 / 4] - base;
    for (uint32_t i = 0; i < count; i++) {
        uint32_t s[0x38 / 4];
        fseek(f, sections + i * 0x38u, SEEK_SET);
        assert(fread(s, 1, sizeof(s), f) == sizeof(s));
        assert((uint64_t)s[1] + s[4] <= sizeof(memory));
        fseek(f, s[3], SEEK_SET);
        assert(fread(memory + s[1], 1, s[4], f) == s[4]);
    }
    fclose(f);
    memcpy(initial, memory, sizeof(memory));
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    load_xbe(argv[1]);
    const uint32_t dev = 0x261240u, stack = 0x700000u;
    uint32_t cases = 0;
    /* Every operation, argument triple, RGB/alpha path and first stage.
     * The expected CPU flags and returned dirty bits come from the actual
     * instruction translation, with jump tables loaded from the XBE. */
    for (uint32_t first = 0; first < 2; first++)
    for (uint32_t alpha = 0; alpha < 2; alpha++)
    for (uint32_t flags = 0; flags < 4; flags++)
    for (uint32_t op = 1; op <= 26; op++)
    for (uint32_t args = 0; args < 216; args++) {
        memcpy(memory + 0x260000, initial + 0x260000, 0x4000);
        MEM32(dev) = 0x600000; MEM32(dev + 4) = 0x610000;
        MEM32(dev + 8) = (flags & 1 ? 0x40u : 0u) | (flags & 2 ? 0x10000u : 0u);
        MEM32(dev + DEV_PIXEL_SHADER) = 0;
        MEM32(0x261178) = first;
        for (uint32_t s = 0; s < 4; s++) {
            MEM32(0x260DD0 + s * 0x80) = 1;
            MEM32(0x260DE0 + s * 0x80) = 1;
        }
        uint32_t state = 0x260DA0 + (first ? 3 : 0) * 0x80;
        if (alpha) MEM32(state + 0x30) = 2;
        MEM32(state + 0x30 + alpha * 0x10) = op;
        MEM32(state + 0x34 + alpha * 0x10) = args % 6;
        MEM32(state + 0x38 + alpha * 0x10) = (args / 6) % 6;
        MEM32(state + 0x3C + alpha * 0x10) = args / 36;
        uint32_t before = MEM32(dev + 8);
        esp = stack; MEM32(esp + 4) = dev; MEM32(esp + 8) = 0x800;
        sub_0025A9D0();
        assert(esp == stack + 12);
        uint32_t expected_flags = MEM32(dev + 8), expected_dirty = eax;
        MEM32(dev + 8) = before;
        uint32_t actual_dirty = flush_combiners(dev, 0x800);
        if (MEM32(dev + 8) != expected_flags || actual_dirty != expected_dirty) {
            fprintf(stderr, "FAIL first=%u alpha=%u op=%u args=%u flags=%08X/%08X dirty=%08X/%08X\n",
                    first, alpha, op, args, MEM32(dev + 8), expected_flags, actual_dirty, expected_dirty);
            return 1;
        }
        cases++;
    }
    printf("PASS: %u combiner CPU-state cases against original XBE instruction translation\n", cases);
    cases = 0;
    for (uint32_t stage = 0; stage < 4; stage++)
    for (uint32_t mag = 0; mag <= 5; mag++)
    for (uint32_t min = 0; min <= 5; min++)
    for (uint32_t anis = 0; anis <= 16; anis++)
    for (uint32_t swizzle = 0; swizzle < 2; swizzle++) {
        memcpy(memory + 0x260000, initial + 0x260000, 0x4000);
        MEM32(dev) = 0x600000; MEM32(dev + 4) = 0x610000;
        uint32_t state = 0x260DA4 + stage * 0x80;
        MEM32(state + 8) = min; MEM32(state + 12) = mag;
        MEM32(state + 28) = anis;
        MEM32(dev + 0xF88 + stage * 4) = swizzle;
        esp = stack; MEM32(esp + 4) = dev; MEM32(esp + 8) = 1u << stage;
        sub_00256800();
        assert(esp == stack + 12);
        uint32_t expected = MEM32(dev + 0x774 + stage * 4);
        MEM32(dev + 0x774 + stage * 4) = 0;
        MEM32(DIRTY) = (1u << stage) | 0xC0000070u;
        native_flush_state(dev);
        assert(MEM32(dev + 0x774 + stage * 4) == expected);
        assert(MEM32(DIRTY) == 0xC0000070u);
        cases++;
    }
    printf("PASS: %u filter CPU-state cases against original instruction translation\n", cases);
    cases = 0;
    for (uint32_t shader_flags = 0; shader_flags < 16; shader_flags++)
    for (uint32_t sources = 0; sources < 1024; sources++) {
        memcpy(memory + 0x260000, initial + 0x260000, 0x4000);
        MEM32(0x261238) = dev;
        MEM32(dev + DEV_VERTEX_SHADER) = 0x500000;
        MEM32(0x500004) = shader_flags << 10;
        MEM32(0x261138) = 1; MEM32(0x261144) = sources & 1;
        MEM32(dev + 0x1928) = 0;
        uint32_t v = sources;
        for (uint32_t i = 0; i < 8; i++) {
            v = v * 1664525u + 1013904223u;
            MEM32(0x261148 + i * 4) = v >> 30;
        }
        esp = stack;
        sub_00256270();
        uint32_t expected = eax;
        MEM32(DIRTY) = 0x1000;
        native_flush_state(dev);
        assert(MEM32(dev + 0x7D0) == expected);
        assert(MEM32(DIRTY) == 0);
        cases++;
    }
    printf("PASS: %u material-mask cases against original instruction translation\n", cases);
    return 0;
}
