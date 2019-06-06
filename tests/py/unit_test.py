import unittest
from os.path import join, dirname, abspath

source_path = abspath(join(dirname(__file__), '../resources/test1.png'))
# size of chunk + chunk header + chunk length + crc
text_chunk_size = 25+4+4+4


class ExtensionTest(unittest.TestCase):
    def test_remove_chunks(self):
        from png_debloat import remove_chunks

        with open(source_path, 'rb') as f:
            f.seek(0, 2)
            size_before = f.tell()
            f.seek(0)
            res = remove_chunks(f, ['tEXt'])

        self.assertEqual(len(res.read()), size_before - text_chunk_size)
