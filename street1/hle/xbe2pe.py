"""Wrap an Xbox XBE image as a PE32 (i386) so PE-only tools (REA/Ghidra P0) can load it.

Every XBE section is placed at its original virtual address with image base 0x10000,
so addresses in the PE match guest VAs exactly. No imports/relocations are emitted;
the kernel thunk table stays as plain data. The output is an analysis artifact only.
"""
import struct
import sys

FILE_ALIGN = 0x20
SECT_ALIGN = 0x20  # low-alignment PE: file offset == RVA


def align(v, a):
    return (v + a - 1) & ~(a - 1)


def main(src, dst):
    b = open(src, "rb").read()
    base = struct.unpack_from("<I", b, 0x104)[0]
    entry = struct.unpack_from("<I", b, 0x128)[0]
    nsec, sec_addr = struct.unpack_from("<II", b, 0x11C)
    # Retail entry point is XORed with a key.
    for key in (0xA8FC57AB, 0x94859D4B):
        e = entry ^ key
        if base <= e < base + 0x4000000:
            entry = e
            break
    sections = []
    for i in range(nsec):
        o = sec_addr - base + 56 * i
        flags, va, vsize, raw, rsize, name_addr = struct.unpack_from("<IIIIII", b, o)
        n = b[name_addr - base:name_addr - base + 16].split(b"\0")[0].decode("ascii", "replace")
        sections.append((n, flags, va, vsize, raw, rsize))
    sections.sort(key=lambda s: s[2])

    # PE: headers occupy [base, first section VA). XBE header itself is mapped there too.
    first_va = sections[0][2]
    hdr_size = first_va - base
    nsect_pe = len(sections)
    pe_off = 0x80
    opt_size = 0xE0
    sect_tab = pe_off + 24 + opt_size
    headers_raw = align(sect_tab + 40 * nsect_pe, FILE_ALIGN)

    out_sections = []
    blobs = []
    for (n, flags, va, vsize, raw, rsize) in sections:
        data = b[raw:raw + min(rsize, vsize)]
        size_raw = len(data)
        raw_ptr = va - base
        ch = 0x40000000  # readable
        if flags & 0x4:
            ch |= 0x20000000 | 0x20  # executable code
        else:
            ch |= 0x40
        if flags & 0x1:
            ch |= 0x80000000  # writable
        out_sections.append((n[:8], vsize, va - base, size_raw, raw_ptr, ch))
        blobs.append((raw_ptr, data))
    nsect_pe = len(out_sections)
    last = sections[-1]
    image_size = align(last[2] + max(last[3], last[5]) - base, SECT_ALIGN)

    hdr = bytearray(headers_raw)
    hdr[0:2] = b"MZ"
    struct.pack_into("<I", hdr, 0x3C, pe_off)
    struct.pack_into("<4sHHIIIHH", hdr, pe_off, b"PE\0\0", 0x14C, nsect_pe, 0, 0, 0, opt_size, 0x0102 | 0x0001)
    o = pe_off + 24
    struct.pack_into("<HBBIIIIIIIIIHHHHHHIIIIHHIIIIII", hdr, o,
                     0x10B, 1, 0, 0, 0, 0, entry - base, first_va - base, 0, base,
                     SECT_ALIGN, FILE_ALIGN, 4, 0, 0, 0, 4, 0, 0, image_size, headers_raw, 0,
                     3, 0, 0x100000, 0x1000, 0x100000, 0x1000, 0, 16)
    for i, (n, vsize, rva, size_raw, ptr, ch) in enumerate(out_sections):
        struct.pack_into("<8sIIIIIIHHI", hdr, sect_tab + 40 * i, n.encode("ascii", "replace"),
                         vsize, rva, size_raw, ptr, 0, 0, 0, 0, ch)
    out = bytearray(align(max(p + len(d) for p, d in blobs), FILE_ALIGN))
    out[:len(hdr)] = hdr
    for ptr, data in blobs:
        out[ptr:ptr + len(data)] = data
    open(dst, "wb").write(out)
    print(f"entry=0x{entry:08X} base=0x{base:08X} sections={nsect_pe} size={len(out)}")
    for s in out_sections:
        print(f"  {s[0]:8s} va=0x{s[2] + base:08X} vsize=0x{s[1]:X}")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
