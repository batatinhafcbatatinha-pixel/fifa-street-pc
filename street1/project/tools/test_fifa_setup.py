import hashlib
import io
from pathlib import Path
import struct
import tempfile
import unittest
import fifa_setup as setup


class ExtractionTests(unittest.TestCase):
    def disc_tree(self, name='hello.txt', extent=2):
        data = bytearray(8192)
        encoded = name.encode('ascii')
        struct.pack_into('<HHIIBB', data, 2048, 0, 0, extent, 5, 32, len(encoded))
        data[2062:2062+len(encoded)] = encoded
        data[4096:4101] = b'hello'
        disc = object.__new__(setup.Disc)
        disc.file = io.BytesIO(data)
        disc.length, disc.base = len(data), 0
        disc.entries, disc.directories, disc.names = [], set(), set()
        return disc

    def test_extract_and_hash(self):
        disc = self.disc_tree()
        disc.walk(1, 2048, '', 0)
        with tempfile.TemporaryDirectory() as folder:
            target = Path(folder) / 'new'
            records = disc.extract(target)
            self.assertEqual((target / 'hello.txt').read_bytes(), b'hello')
            self.assertEqual(records[0]['sha256'], hashlib.sha256(b'hello').hexdigest())
            with self.assertRaises(FileExistsError):
                disc.extract(target)

    def test_path_traversal_rejected(self):
        for name in ('..', '../bad', 'C:bad', 'NUL.txt', 'bad.', 'a\\b'):
            with self.subTest(name=name), self.assertRaises(ValueError):
                self.disc_tree(name).walk(1, 2048, '', 0)

    def test_extent_rejected(self):
        with self.assertRaises(ValueError):
            self.disc_tree(extent=99).walk(1, 2048, '', 0)

    def test_tree_cycle_rejected(self):
        disc = self.disc_tree()
        raw = bytearray(disc.file.getvalue())
        struct.pack_into('<H', raw, 2048, 1)  # Child offset 4: invalid/cyclic entry.
        disc.file = io.BytesIO(raw)
        with self.assertRaises(ValueError):
            disc.walk(1, 2048, '', 0)

    def test_wrong_title_rejected(self):
        data = bytearray(0x12000)
        data[0x10000:0x10014] = b'MICROSOFT*XBOX*MEDIA'
        struct.pack_into('<II', data, 0x10014, 33, 2048)
        struct.pack_into('<HHIIBB', data, 33*2048, 0, 0, 34, 512, 32, 11)
        data[33*2048+14:33*2048+25] = b'default.xbe'
        offset = 34*2048
        data[offset:offset+4] = b'XBEH'
        struct.pack_into('<I', data, offset+0x104, 0x10000)
        struct.pack_into('<I', data, offset+0x118, 0x10140)
        struct.pack_into('<I', data, offset+0x148, 0x12345678)
        with tempfile.TemporaryDirectory() as folder:
            image = Path(folder) / 'wrong.iso'
            image.write_bytes(data)
            with self.assertRaisesRegex(ValueError, 'Wrong game'):
                setup.Disc(image)


if __name__ == '__main__':
    unittest.main()
